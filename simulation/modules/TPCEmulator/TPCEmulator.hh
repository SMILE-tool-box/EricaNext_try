#ifndef EricaSim_TPCEmulator_H
#define EricaSim_TPCEmulator_H 1

#include <memory>
#include <string>
#include <vector>
#include <anlnext/BasicModule.hh>

#include "FiducialModify.h"

class TF1;
class TH2D;
class TH2F;
class TRandom3;
class CalcCluster;

namespace ericasim {

class EdepPickUp;

// Port of SMILE_tool_box_v2.20 TPC_emulator (Erica/TPC_emulator, ver 0.41, Y. Nakamura) to ANLNext.
// From the TPC energy deposits of the event (EdepPickUp: every Geant4 step, keV, world mm):
//   ionisation (cluster-size data or Poisson x Fano) -> drift with diffusion -> strip / clock
//   (analytic field correction of FiducialModify, GEM gap) -> GEM (discrete) x uPIC (Polya, gain from
//   the gain map) amplification -> Iwaki response template -> strip x clock signal map ->
//   per board: hit records (clock + 4 x 32 strip bits over Vth) and 4 x 512 FADC samples.
// The output is what TPC_FileManager writes to uTPC_anode/cathode raw files (RawWriter2p).
// Physics, constants and quirks are those of the reference; differences:
//  - input from EdepPickUp instead of the haboc banks TPC_Hit_E/X/Y/Z, uPIC_Size, DriftOffset, DataFormat
//    (here: parameters upic_size, drift_offset, daq_mode)
//  - random numbers: TRandom3 seeded from random_seed (the reference used the default seed 4357 for its
//    TRandom3s and gRandom for the uPIC Polya gain), so results agree statistically, not bit-wise
//  - no diagnostic histograms
// Put it after SimTrigger.
class TPCEmulator : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(TPCEmulator, 0.41);
public:
  TPCEmulator();
  ~TPCEmulator();
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;

  // output of the current event (boards 0 .. NBoard()-1)
  int NBoard() const { return nEnc_; }
  unsigned EventID() const { return eventID_; }
  unsigned EventTime() const { return eventTime_; }
  // side 0 anode, 1 cathode: hit records, 5 words each (clock, 4 x 32 strip bits)
  const std::vector<unsigned>& Hits(int side, int board) const { return hits_[side][board]; }
  int NHit(int side, int board) const { return nhit_[side][board]; }
  // 4 ch x 512 samples of a board
  const unsigned short* FADC(int side, int board) const { return &fadc_[side][(size_t)board * 4 * 512]; }
  double DriftVelocity() const { return driftV_; }

private:
  bool GEMGapCheck(double x, double y) const;   // false: in the GEM gap
  bool GEMGapCheck(int anode, int cathode) const;

  const EdepPickUp* edep_ = nullptr;

  // parameters (names of the reference macro)
  double wValue_, fano_, driftV_, trans_, longi_, pitch_, gemGain_;
  std::string fnameIwakiTemp_, fnameGainMap_, fnameCluster_;
  double baselineFluc_;
  int vthA_, vthC_, dacSurvey_;
  int offsetForTPC_, offsetForETCC_;
  double gainModAmp_;
  int debugMode_, calcModeCluster_, fiducialModify_;
  double upicSize_;      // [cm] (SMILE_2p uPIC_Size)
  double driftOffset_;   // [cm] (SMILE_2p DriftOffset)
  int daqMode_;          // SMILE_2p DataFormat (DAQ_Mode)
  int randomSeed_;
  double polyaUPIC_ = 0.65, polyaGEM_ = 0.5;
  double stripWidth_ = 38.29, stripGap_ = 0.5;

  // derived
  int nEnc_ = 3;
  double upic_ = 0., tpcOffset_ = 0., vthAV_ = 0., vthCV_ = 0.;
  std::vector<double> upicGain_;   // [aid * nCh + cid]
  int offsetETCCUsed_ = 0;

  std::unique_ptr<TRandom3> rndm_, rndmGEM_, rndmGain_;
  std::unique_ptr<CalcCluster> cluster_;
  std::unique_ptr<TF1> fcnGain_;
  std::vector<double> gemIntegral_;   // DiscreateGain
  double gemMean_ = 20.;
  std::unique_ptr<TH2F> iwakiTemp_;
  std::unique_ptr<TH2D> seedXZ_, seedYZ_;
  std::unique_ptr<TF1> etccNoise_[4];
  double ll_[4] = {42, 46, 48, 50}, ul_[4] = {46, 48, 50, 512};
  FiducialModify fidmod_;
  double startTime_ = -1.;

  // event output
  unsigned eventID_ = 0, eventTime_ = 0;
  std::vector<unsigned> hits_[2][16];
  int nhit_[2][16] = {};
  std::vector<unsigned short> fadc_[2], bufFadc_[2];
};

} // namespace ericasim

#endif
