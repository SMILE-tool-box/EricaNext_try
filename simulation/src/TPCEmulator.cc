#include "TPCEmulator.hh"
#include "EdepPickUp.hh"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

#include <TF1.h>
#include <TFile.h>
#include <TH2D.h>
#include <TH2F.h>
#include <TMath.h>
#include <TRandom3.h>
#include <TTimeStamp.h>

#include "CalcCluster.h"

using namespace anlnext;

namespace ericasim {

namespace {
const double UPIC_GAIN_DEFAULT = 1000;
const int MODE_CLUSTER_DATA = 0;
}

TPCEmulator::TPCEmulator()
  // TPC_emulator::TPC_emulator() defaults
  : wValue_(26.4 * 0.95 + 54. * 0.03 + 26. * 0.02), fano_(0.20), driftV_(35.0), trans_(0.26), longi_(0.24),
    pitch_(0.8), gemGain_(20), baselineFluc_(21.481), vthA_(7500), vthC_(10500), dacSurvey_(9000),
    offsetForTPC_(85), offsetForETCC_(85), gainModAmp_(1), debugMode_(0), calcModeCluster_(MODE_CLUSTER_DATA),
    fiducialModify_(1), upicSize_(30.72), driftOffset_(3), daqMode_(3), randomSeed_(4357)
{
}

TPCEmulator::~TPCEmulator() = default;

ANLStatus TPCEmulator::mod_define()
{
  define_parameter("w_value", &mod_class::wValue_);
  set_parameter_description("W value [eV]");
  define_parameter("Fano_Factor", &mod_class::fano_);
  define_parameter("DriftVel", &mod_class::driftV_);
  set_parameter_description("drift velocity [um/ns]");
  define_parameter("DiffTrans", &mod_class::trans_);
  set_parameter_description("transverse diffusion [mm/sqrt(cm)]");
  define_parameter("DiffLong", &mod_class::longi_);
  set_parameter_description("longitudinal diffusion [mm/sqrt(cm)]");
  define_parameter("ReadOutPitch", &mod_class::pitch_);
  define_parameter("GEM_Gain", &mod_class::gemGain_);
  define_parameter("IwakiTemplate", &mod_class::fnameIwakiTemp_);
  define_parameter("GainMap", &mod_class::fnameGainMap_);
  set_parameter_description("TPC gain map (Calibration*.dat: anode ch, cathode ch, a, b)");
  define_parameter("ClusterData", &mod_class::fnameCluster_);
  define_parameter("BaseLineFluctuation", &mod_class::baselineFluc_);
  define_parameter("Vth_Anode", &mod_class::vthA_);
  define_parameter("Vth_Cathode", &mod_class::vthC_);
  define_parameter("DAC_Survey", &mod_class::dacSurvey_);
  define_parameter("Offset_for_TPC", &mod_class::offsetForTPC_);
  define_parameter("Offset_for_ETCC", &mod_class::offsetForETCC_);
  define_parameter("Gain_Modify_Amp", &mod_class::gainModAmp_);
  define_parameter("Debug_Mode", &mod_class::debugMode_);
  define_parameter("Calc_Mode_Cluster", &mod_class::calcModeCluster_);
  set_parameter_description("0: cluster-size data (ClusterData), 1: Poisson x Fano");
  define_parameter("Fiducial_Modify_Flag", &mod_class::fiducialModify_);
  set_parameter_description("1: analytic drift-field correction (FiducialModify functions)");
  define_parameter("upic_size", &mod_class::upicSize_);
  set_parameter_description("uPIC size [cm] (SMILE_2p uPIC_Size)");
  define_parameter("drift_offset", &mod_class::driftOffset_);
  set_parameter_description("[cm] (SMILE_2p tpc_offset)");
  define_parameter("daq_mode", &mod_class::daqMode_);
  set_parameter_description("SMILE_2p DAQ_Mode: 3 adds the ETCC-mode noise, 1 shifts to the TPC trigger clock");
  define_parameter("random_seed", &mod_class::randomSeed_);
  return AS_OK;
}

ANLStatus TPCEmulator::mod_initialize()
{
  get_module("EdepPickUp", &edep_);

  rndm_ = std::make_unique<TRandom3>(randomSeed_);
  rndmGEM_ = std::make_unique<TRandom3>(randomSeed_ + 1);
  rndmGain_ = std::make_unique<TRandom3>(randomSeed_ + 2);

  // TPC_emulator::Init()
  upic_ = upicSize_ * 10.;
  nEnc_ = (int)TMath::Ceil(upic_ / pitch_ / 128);
  const int nCh = nEnc_ * 4;

  if (calcModeCluster_ == MODE_CLUSTER_DATA) {
    cluster_ = std::make_unique<CalcCluster>(fnameCluster_);
    cluster_->SetSeed(randomSeed_ + 3);
  }

  offsetETCCUsed_ = offsetForETCC_ - 5;   // average of "delay" = 5
  tpcOffset_ = driftOffset_ * 10.;

  vthAV_ = (dacSurvey_ - vthA_) / TMath::Power(2., 13) * 3.3;
  vthCV_ = (vthC_ - dacSurvey_) / TMath::Power(2., 13) * 3.3;

  upicGain_.assign((size_t)nCh * nCh, 0.);
  std::ifstream ifsGain(fnameGainMap_);
  if (!ifsGain) {
    std::cerr << "TPCEmulator: gain map not found: " << fnameGainMap_ << std::endl;
    return AS_QUIT_ERROR;
  }
  int aid, cid;
  double parA, parB;
  while (true) {
    ifsGain >> aid >> cid >> parA >> parB;
    if (ifsGain.eof()) break;
    const double calA = 1. / gainModAmp_ * parA;
    upicGain_[(size_t)aid * nCh + cid] = (wValue_ * 1e-3 / 1.602e-7 / 700. / calA) / gemGain_;
  }

  seedXZ_ = std::make_unique<TH2D>("TPCEmu_SeedMap_XZ", "", (int)(upic_ / pitch_), -0.5, upic_ / pitch_ - 0.5,
                                   1024, -0.5, 1023.5);
  seedYZ_ = std::make_unique<TH2D>("TPCEmu_SeedMap_YZ", "", (int)(upic_ / pitch_), -0.5, upic_ / pitch_ - 0.5,
                                   1024, -0.5, 1023.5);
  seedXZ_->SetDirectory(nullptr);
  seedYZ_->SetDirectory(nullptr);

  fcnGain_ = std::make_unique<TF1>("TPCEmu_fcn_gain", "TMath::Power((1+[0])*x/[1], [0])*TMath::Exp(-(1+[0])*x/[1])",
                                   0, 1e6);
  fcnGain_->SetNpx(1e6);
  fcnGain_->SetParameters(polyaUPIC_, UPIC_GAIN_DEFAULT);

  // DiscreateGain::Set(polya_gem, gem_gain): geometric distribution, Npx = 1e4 + 1
  gemMean_ = gemGain_;
  const int npx = 10001;
  gemIntegral_.assign(npx, 0.);
  for (int i = 1; i < npx; i++)
    gemIntegral_[i] = gemIntegral_[i - 1] + 1. / gemMean_ * std::pow(1. - 1. / gemMean_, i - 1);

  iwakiTemp_ = std::make_unique<TH2F>("TPCEmu_IwakiTemp", "", 2000, -0.5, 1999.5, 2500, -0.5, 2499.5);
  iwakiTemp_->SetDirectory(nullptr);
  {
    TFile tempFile(fnameIwakiTemp_.c_str());
    auto t = dynamic_cast<TH2F*>(tempFile.FindObjectAny("iwaki_template"));
    if (t == nullptr) {
      std::cerr << "TPCEmulator: no iwaki_template in " << fnameIwakiTemp_ << std::endl;
      return AS_QUIT_ERROR;
    }
    iwakiTemp_->Add(t);
  }

  // ETCC-mode noise
  const double par[4][5] = {{14.39, 42.0, 17.01, 3.446, -0.317},
                            {39.86, 46.0, 20.77, 3.522, -0.909},
                            {16.98, 48.0, 17.22, 6.009, -6.590},
                            {14.40, 50.0, 18.29, 8.288, -0.671}};
  for (int i = 0; i < 4; i++) {
    etccNoise_[i] = std::make_unique<TF1>(Form("TPCEmu_ETCC_Noise%d", i),
                                          "[0]* TMath::Exp( -(x-[1])/[2] ) * TMath::Sin( (x/[3]-[4])*2*TMath::Pi() )",
                                          ll_[i], ul_[i]);
    for (int j = 0; j < 5; j++) etccNoise_[i]->SetParameter(j, par[i][j]);
  }

  for (int s = 0; s < 2; s++) {
    for (int b = 0; b < nEnc_; b++) hits_[s][b].assign(1024 * 5, 0u);
    fadc_[s].assign((size_t)nEnc_ * 4 * 512, 0);
    bufFadc_[s].assign((size_t)nEnc_ * 4 * 512, 0);
  }
  std::cout << "TPCEmulator: " << nEnc_ << " boards, GEM polya " << polyaGEM_ << " gain " << gemGain_
            << ", uPIC polya " << polyaUPIC_ << std::endl;
  return AS_OK;
}

ANLStatus TPCEmulator::mod_analyze()
{
  TTimeStamp now;
  if (startTime_ < 0) startTime_ = now.AsDouble();

  const std::vector<double>& trackE = edep_->TPCStepE();
  const std::vector<double>& trackX = edep_->TPCStepX();
  const std::vector<double>& trackY = edep_->TPCStepY();
  const std::vector<double>& trackZ = edep_->TPCStepZ();
  const int nData = (int)trackE.size();

  seedXZ_->Reset();
  seedYZ_->Reset();

  const double delay = rndm_->Uniform(10);
  rndm_->Gaus(1, 0.07);                       // r_gem (drawn but not used in the reference)
  const double rUPIC = rndm_->Gaus(1, 0.10);
  const int nCh = nEnc_ * 4;

  for (int i = 0; i < nData; i++) {
    int nSeed;
    if (calcModeCluster_ == MODE_CLUSTER_DATA) {
      nSeed = cluster_->GetClustSize(1e3 * trackE[i]);
    }
    else {
      const double fNtemp = rndm_->Poisson(1e3 * trackE[i] / wValue_ / fano_);
      nSeed = (int)TMath::Floor(fNtemp * fano_);
    }

    for (int e = 0; e < nSeed; e++) {
      const double dx = rndm_->Gaus(0, trans_ * TMath::Sqrt(trackZ[i] / 10.));   // mm
      const double dy = rndm_->Gaus(0, trans_ * TMath::Sqrt(trackZ[i] / 10.));
      const double dz = rndm_->Gaus(0, longi_ * TMath::Sqrt(trackZ[i] / 10.));

      const double x = trackX[i] + dx;
      const double y = trackY[i] + dy;
      double z = trackZ[i] + dz - tpcOffset_;

      double xi = (x + upic_ / 2.) / pitch_;
      double yi = (y + upic_ / 2.) / pitch_;
      double zi = TMath::Abs(z) / driftV_ * 1000 + offsetETCCUsed_ * 10;

      if (fiducialModify_ == 1) {
        z += tpcOffset_;
        fidmod_.RealToDetector_3D(x, y, z, &xi, &yi, &zi, driftV_);
        z -= tpcOffset_;
        zi *= 1000;   // [us] -> [ns]
        zi += offsetETCCUsed_ * 10;
        if (!GEMGapCheck((int)xi, (int)yi)) continue;
      }
      else {
        if (!GEMGapCheck(xi, yi)) continue;
      }

      if (xi < 0 || 128 * nEnc_ <= xi || yi < 0 || 128 * nEnc_ <= yi) continue;

      const int xch = (int)(xi / (128 / 4));
      const int ych = (int)(yi / (128 / 4));

      // DiscreateGain::GetRand()
      const double r = rndmGEM_->Uniform(1);
      const int gGEM = (int)(std::lower_bound(gemIntegral_.begin(), gemIntegral_.end(), r) - gemIntegral_.begin());
      double gUPIC = 0;
      for (int k = 0; k < gGEM; k++)
        gUPIC += fcnGain_->GetRandom(rndmGain_.get()) * upicGain_[(size_t)xch * nCh + ych] / UPIC_GAIN_DEFAULT * rUPIC;
      gUPIC /= gGEM;
      const int wIndex = (int)rndm_->Uniform(2000);

      for (int t = 0; t < 250; t++) {
        const double tOffset = TMath::Floor(zi);
        const double wtTime = delay + (zi - tOffset) + t * 10;
        const double time = tOffset / 10. + t;
        if (wtTime > 2500 || time > 1024) break;
        const double w = iwakiTemp_->GetBinContent(wIndex + 1, (int)wtTime);
        seedXZ_->Fill(xi, time, gGEM * gUPIC * w);
        seedYZ_->Fill(yi, time, gGEM * gUPIC * w);
      }
    }
  }

  // hit records and FADC samples
  TTimeStamp now2;
  const double dTime = now2.AsDouble() - startTime_;
  eventID_ = (unsigned)edep_->EventID();
  eventTime_ = (unsigned)(dTime * 1e2);   // reference: wall-clock time since the start x 100
  int trigClk = -1;
  for (int s = 0; s < 2; s++)
    for (int b = 0; b < nEnc_; b++) nhit_[s][b] = 0;
  unsigned short* fa = fadc_[0].data();
  unsigned short* fc = fadc_[1].data();
  const double thrA = vthAV_ * 1e3 / 150, thrC = vthCV_ * 1e3 / 150;

  for (int j = 0; j < 1024; j++) {
    for (int i = 0; i < nEnc_; i++) {
      unsigned* hA = hits_[0][i].data();
      unsigned* hC = hits_[1][i].data();
      hA[5 * nhit_[0][i]] = j;
      hC[5 * nhit_[1][i]] = j;
      bool anodeHit = false, cathodeHit = false;
      for (int ch = 0; ch < 4; ch++) {
        hA[5 * nhit_[0][i] + ch + 1] = 0;
        hC[5 * nhit_[1][i] + ch + 1] = 0;
        const double whiteA = rndm_->Gaus(0, baselineFluc_);
        const double whiteC = rndm_->Gaus(0, baselineFluc_);
        // reference: 33 strip bins (one of the next chip included)
        const double mVA = seedXZ_->Integral(128 * i + 32 * ch + 1, 128 * i + 32 * ch + 33, j + 1, j + 1) + whiteA;
        const double mVC = seedYZ_->Integral(128 * i + 32 * ch + 1, 128 * i + 32 * ch + 33, j + 1, j + 1) + whiteC;

        double etccNoise = 0;   // adc
        if (daqMode_ == 3)
          for (int k = 0; k < 4; k++) {
            const double clk = (double)j / 2 - delay / 10 / 2;
            if (clk > ll_[k] && clk <= ul_[k]) etccNoise = etccNoise_[k]->Eval(clk);
          }

        if (mVA > thrA) {
          for (int strip = 0; strip < 32; strip++)
            if (seedXZ_->GetBinContent(128 * i + 32 * ch + strip + 1, j + 1) + (etccNoise + whiteA) / 32 > thrA) {
              if (trigClk == -1) trigClk = j;
              hA[5 * nhit_[0][i] + ch + 1] += (0x1u << (31 - strip));
              anodeHit = true;
            }
        }
        if (j % 2) {
          const int adc = (int)TMath::Floor(mVA / 2000. * 1024 / 2 + etccNoise);
          if (adc + 40 < 1024) fa[(4 * i + ch) * 512 + j / 2] = (adc + 40) & 0xffff;
          else                 fa[(4 * i + ch) * 512 + j / 2] = (1023) & 0xffff;
        }

        if (mVC > thrC) {
          for (int strip = 0; strip < 32; strip++)
            if (seedYZ_->GetBinContent(128 * i + 32 * ch + strip + 1, j + 1) + (etccNoise + whiteC) / 32 > thrC) {
              if (trigClk == -1) trigClk = j;
              hC[5 * nhit_[1][i] + ch + 1] += (0x1u << (31 - strip));
              cathodeHit = true;
            }
        }
        if (j % 2) {
          const int adc = TMath::FloorNint(mVC / 2000. * 1024 / 2 + etccNoise);
          if (adc + 40 < 1024) fc[(4 * i + ch) * 512 + j / 2] = (1023 - adc - 40) & 0xffff;
          else                 fc[(4 * i + ch) * 512 + j / 2] = 0;
        }
      }
      if (anodeHit) nhit_[0][i]++;
      if (cathodeHit) nhit_[1][i]++;
    }
  }

  // TPC mode (DAQ_Mode 1): shift to the trigger clock (Y. Nakamura)
  if (daqMode_ == 1 && debugMode_ == 0) {
    const int clkDiff = trigClk - offsetForTPC_;
    std::copy(fadc_[0].begin(), fadc_[0].end(), bufFadc_[0].begin());
    std::copy(fadc_[1].begin(), fadc_[1].end(), bufFadc_[1].begin());
    for (int i = 0; i < nEnc_; i++) {
      for (int j = 0; j < 1024; j++) {
        if (j < nhit_[0][i]) hits_[0][i][5 * j] -= clkDiff;
        if (j < nhit_[1][i]) hits_[1][i][5 * j] -= clkDiff;
        for (int ch = 0; ch < 4; ch++) {
          if (j % 2) {
            fa[(4 * i + ch) * 512 + j / 2] = bufFadc_[0][(4 * i + ch) * 512 + ((j + clkDiff) % 1024) / 2];
            fc[(4 * i + ch) * 512 + j / 2] = bufFadc_[1][(4 * i + ch) * 512 + ((j + clkDiff) % 1024) / 2];
          }
        }
      }
    }
  }
  return AS_OK;
}

bool TPCEmulator::GEMGapCheck(double x, double y) const
{
  (void)y;
  for (int i = 0; i < 4; i++) {
    if (-stripGap_ / 2. + (stripWidth_ + stripGap_) * i < std::abs(x) &&
        std::abs(x) < stripGap_ / 2. + (stripWidth_ + stripGap_) * i)
      return false;
  }
  return true;
}

bool TPCEmulator::GEMGapCheck(int anode, int cathode) const
{
  (void)cathode;
  for (int i = 0; i < 4; i++) {
    const int stripNumber = (int)(stripWidth_ * i / 0.8);
    if (std::abs(anode - (128 * 3 / 2)) == stripNumber) return false;
  }
  return true;
}

} // namespace ericasim
