#include "HAEmulator.hh"
#include "EdepPickUp.hh"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

#include <TMath.h>
#include <TRandom3.h>

#include "HA_common.h"

using namespace anlnext;
using namespace HA_common;

namespace ericasim {

namespace {
const double HA_RELATIVE_GAIN_DEFAULT = 3.1;
const double HA_THRESHOLD = 33.;
}

HAEmulator::HAEmulator() : randomSeed_(4357) {}
HAEmulator::~HAEmulator() = default;

ANLStatus HAEmulator::mod_define()
{
  define_parameter("ChargeDivTemp", &mod_class::chargeDivFile_);
  set_parameter_description("charge division of the resistor chain (charge.div)");
  define_parameter("PMT_MapDir", &mod_class::pmtMapDir_);
  define_parameter("E_Res_662", &mod_class::resolutionFile_);
  set_parameter_description("pixel peak / resolution file (Pixel_Resolution_*.dat), used for Cal_HA*.dat");
  define_parameter("cal_output_dir", &mod_class::calOutputDir_);
  set_parameter_description("where to write Cal_HA<name>.dat (empty: not written)");
  define_parameter("random_seed", &mod_class::randomSeed_);
  return AS_OK;
}

ANLStatus HAEmulator::mod_initialize()
{
  get_module("EdepPickUp", &edep_);
  rndm_ = std::make_unique<TRandom3>(randomSeed_);

  // Emu_HA::Init(): charge division
  std::ifstream chData(chargeDivFile_);
  if (!chData) {
    std::cerr << "HAEmulator: can't find " << chargeDivFile_ << std::endl;
    return AS_QUIT_ERROR;
  }
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++) {
      int raw, col;
      chData >> raw >> col;
      for (int ch = 0; ch < 4; ch++) chData >> ratio_[8 * i + j][ch];
    }

  // Emu_HA::Begin(): pixel peaks, PMT maps, calibration
  std::ifstream resData(resolutionFile_);
  if (!resData) {
    std::cerr << "HAEmulator: can't find " << resolutionFile_ << std::endl;
    return AS_QUIT_ERROR;
  }
  for (int i = 0; i < N_HA; i++)
    for (int j = 0; j < 6; j++)
      for (int ch = 0; ch < 64; ch++) {
        int ha, pmt, x, y;
        double peak, res;
        resData >> ha >> pmt >> x >> y >> peak >> res;
        peak_[i][j][y * 8 + x] = peak;
      }

  map_.assign(N_HA * 6, std::vector<double>(64, 1.));
  for (int hapmt = 0; hapmt < N_HA * 6; hapmt++) {
    std::stringstream ss;
    ss << pmtMapDir_ << "/HA" << std::setw(2) << std::setfill('0') << ids_[hapmt / 6] << "_" << (hapmt % 6) + 1 << ".map";
    std::ifstream mapData(ss.str());
    if (!mapData) std::cerr << "HAEmulator: can't find " << ss.str() << " (uniformity 1)" << std::endl;
    for (int ch = 0; ch < 64; ch++) {
      if (mapData) {
        mapData >> map_[hapmt][ch];
        map_[hapmt][ch] /= 100.;
      }
      else {
        map_[hapmt][ch] = 1.;
      }
    }
  }

  std::vector<double> calA(8 * 8 * 6 * 18, 0.), calB(8 * 8 * 6 * 18, 0.);
  for (int hapmt = 0; hapmt < N_HA * 6; hapmt++) {
    for (int ch = 0; ch < 64; ch++) {
      int ar[8];
      const int calid = hapmt * 64 + ch;
      LeakPxDef(ch, ar);
      double ll[8], sum = 0;
      for (int i = 0; i < 4; i++) {
        ll[i] = leakPlus_;
        ll[i + 4] = leakX_;
        sum += ll[i] + ll[i + 4];
      }
      sum += 1.;
      for (int i = 0; i < 8; i++) ll[i] /= sum;
      calA[calid] = map_[hapmt][ch] / sum;
      for (int i = 0; i < 8; i++)
        if (ar[i] >= 0) calA[calid] += ll[i] * map_[hapmt][ar[i]];
      calA[calid] = 1. / calA[calid] / HA_RELATIVE_GAIN_DEFAULT;   // [ch/keV]
      calA[calid] *= peak_[hapmt / 6][hapmt % 6][ch] / 662.;
    }
  }
  if (!calOutputDir_.empty()) {
    std::filesystem::create_directories(calOutputDir_);
    std::ofstream fout;
    for (int hapmt = 0; hapmt < N_HA * 6; hapmt++) {
      if (hapmt % 6 == 0) {
        char text[512];
        snprintf(text, sizeof(text), "%s/Cal_HA%02d.dat", calOutputDir_.c_str(), Get_HA_ID(hapmt / 6, ID_TO_NAME));
        fout.open(text);
      }
      for (int ch = 0; ch < 64; ch++) {
        fout << ch % 8 << " " << ch / 8 << " " << calA[hapmt * 64 + ch] << " ";
        if (ch % 8 == 7) fout << std::endl;
      }
      fout << std::endl;
      for (int ch = 0; ch < 64; ch++) {
        fout << ch % 8 << " " << ch / 8 << " " << calB[hapmt * 64 + ch] << " ";
        if (ch % 8 == 7) fout << std::endl;
      }
      fout << std::endl;
      if (hapmt % 6 == 5) fout.close();
    }
    std::cout << "HAEmulator: wrote Cal_HA*.dat to " << calOutputDir_ << std::endl;
  }

  deposit_.assign(N_HA * 24, 0.);
  adc_.assign(N_HA * 24, 0);
  return AS_OK;
}

ANLStatus HAEmulator::mod_analyze()
{
  // per pixel, in the order of the first deposit (SMILE_2p::DepositInPSA)
  struct Pixel { int ha, pmt, px; double e; };
  std::vector<Pixel> pixels;
  std::map<std::string, size_t> index;
  const auto& vol = edep_->ScintiStepVolume();
  const auto& es = edep_->ScintiStepE();
  for (size_t k = 0; k < vol.size(); k++) {
    auto it = index.find(vol[k]);
    if (it == index.end()) {
      std::vector<std::string> f;
      std::stringstream ss(vol[k]);
      std::string item;
      while (std::getline(ss, item, '_')) f.push_back(item);
      if (f.size() < 5) continue;
      index[vol[k]] = pixels.size();
      pixels.push_back({std::stoi(f[2]), std::stoi(f[3]), std::stoi(f[4]), es[k]});
    }
    else {
      pixels[it->second].e += es[k];
    }
  }

  const double resolutionSide = 10.5, resolutionBottom = 11.5;
  std::fill(deposit_.begin(), deposit_.end(), 0.);
  for (const Pixel& p : pixels) {
    int ar[8];
    LeakPxDef(p.px, ar);
    double ll[8], sum = 0;
    for (int i = 0; i < 4; i++) {
      ll[i] = rndm_->Gaus(leakPlus_, leakPlusSigma_);
      ll[i + 4] = rndm_->Gaus(leakX_, leakXSigma_);
      sum += ll[i] + ll[i + 4];
    }
    sum += 1;
    for (int i = 0; i < 8; i++) ll[i] = ll[i] / sum;

    const int hapmt = Get_HA_ID(p.ha, NAME_TO_ID) * 6 + p.pmt - 1;
    const double resolution = (Get_HA_ID(p.ha, NAME_TO_ID) < 6) ? resolutionBottom : resolutionSide;
    const double photon = rndm_->Gaus(p.e, 662. * resolution / 100 / 2 / TMath::Sqrt(2 * TMath::Log(2))
                                               * TMath::Sqrt(p.e / 662.));
    double readCharge[4];
    for (int ch = 0; ch < 4; ch++) readCharge[ch] = photon / sum * ratio_[p.px][ch] * map_[hapmt][p.px];
    for (int i = 0; i < 8; i++) {
      const double leak = photon * ll[i];
      if (ar[i] >= 0)
        for (int ch = 0; ch < 4; ch++) readCharge[ch] += leak * ratio_[ar[i]][ch] * map_[hapmt][ar[i]];
    }
    int offset[4];
    const int id = ADCCh(p.ha, p.pmt, offset);
    for (int ch = 0; ch < 4; ch++) deposit_[24 * id + offset[ch]] += readCharge[ch] * HA_RELATIVE_GAIN_DEFAULT;
  }

  eventID_ = (unsigned)edep_->EventID();
  for (int i = 0; i < N_HA * 24; i++) {
    const short buf = (short)(int)TMath::Floor(deposit_[i] + rndm_->Gaus(0, .8));
    if (buf < 4095 && buf > HA_THRESHOLD) adc_[i] = (buf >= 0) ? buf : 0;
    else                                  adc_[i] = 0x2000;
  }
  return AS_OK;
}

int HAEmulator::LeakPxDef(int px, int* ar) const
{
  // Emu_HA::LeakPxDef: neighbours (ar[0..3]: +, ar[4..7]: x), -1 outside the 8 x 8 pixels
  int na = 0;
  for (int i = 0; i < 8; i++) ar[i] = -1;
  switch (px) {
    case 0:  na = 3; ar[0] = 1;  ar[1] = 8;  ar[4] = 9;  break;
    case 7:  na = 3; ar[0] = 6;  ar[1] = 15; ar[4] = 14; break;
    case 56: na = 3; ar[0] = 48; ar[1] = 57; ar[4] = 49; break;
    case 63: na = 3; ar[0] = 55; ar[1] = 62; ar[4] = 54; break;
    case 1: case 2: case 3: case 4: case 5: case 6:
      na = 5; ar[0] = px - 1; ar[1] = px + 1; ar[2] = px + 8; ar[4] = px + 7; ar[5] = px + 9; break;
    case 57: case 58: case 59: case 60: case 61: case 62:
      na = 5; ar[0] = px - 8; ar[1] = px - 1; ar[2] = px + 1; ar[4] = px - 9; ar[5] = px - 7; break;
    case 8: case 16: case 24: case 32: case 40: case 48:
      na = 5; ar[0] = px - 8; ar[1] = px + 1; ar[2] = px + 8; ar[4] = px - 7; ar[5] = px + 9; break;
    case 15: case 23: case 31: case 39: case 47: case 55:
      na = 5; ar[0] = px - 8; ar[1] = px - 1; ar[2] = px + 8; ar[4] = px - 9; ar[5] = px + 7; break;
    default:
      na = 8; ar[0] = px - 8; ar[1] = px - 1; ar[2] = px + 1; ar[3] = px + 8;
      ar[4] = px - 9; ar[5] = px - 7; ar[6] = px + 7; ar[7] = px + 9;
  }
  return na;
}

int HAEmulator::ADCCh(int ha, int pmt, int* offset) const
{
  int index = -1;
  for (int i = 0; i < N_HA; i++)
    if (ids_[i] == ha) { index = i; break; }
  switch (pmt) {   // Emu_HA::ADC_Ch (the active version)
    case 1: offset[0] = 20; offset[1] = 17; offset[2] = 23; offset[3] = 14; break;
    case 2: offset[0] = 0;  offset[1] = 9;  offset[2] = 3;  offset[3] = 6;  break;
    case 3: offset[0] = 8;  offset[1] = 5;  offset[2] = 11; offset[3] = 2;  break;
    case 4: offset[0] = 12; offset[1] = 21; offset[2] = 15; offset[3] = 18; break;
    case 5: offset[0] = 19; offset[1] = 16; offset[2] = 22; offset[3] = 13; break;
    case 6: offset[0] = 1;  offset[1] = 10; offset[2] = 4;  offset[3] = 7;  break;
    default: offset[0] = 20; offset[1] = 17; offset[2] = 14; offset[3] = 23; break;
  }
  return index;
}

} // namespace ericasim
