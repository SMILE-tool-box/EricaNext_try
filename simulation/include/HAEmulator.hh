#ifndef EricaSim_HAEmulator_H
#define EricaSim_HAEmulator_H 1

#include <memory>
#include <string>
#include <vector>
#include <anlnext/BasicModule.hh>

class TRandom3;

namespace ericasim {

class EdepPickUp;

// Port of SMILE_tool_box_v2.20 Emu_HA (Erica/Emu_HA) to ANLNext.
// From the scintillator deposits of the event (EdepPickUp, summed per pixel volume
// scinti_<bottom|side>_<HA>_<PMT>_<pixel> in the order of the first deposit, as SMILE_2p::DepositInPSA):
//   energy resolution (bottom 11.5 %, side 10.5 % FWHM at 662 keV, sqrt(E)), light leak to the
//   neighbouring pixels, charge division of the resistor chain (charge.div), PMT anode uniformity
//   (PMT_Map, 1 where the map is missing), gain 3.1 ch/keV, pedestal noise 0.8 ch,
//   threshold 33 / overflow 4095 -> 0x2000 -> 24 ADC values per HA (HA_FileManager order).
// Also writes the HA calibration files Cal_HA<name>.dat (as Emu_HA::Begin) to cal_output_dir, for
// reconstructing the simulated raw data with HA_Calc.
// Differences: input from EdepPickUp instead of the haboc ScintiHit banks; TRandom3 seeded from
// random_seed (the reference: default seed 4357).
class HAEmulator : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(HAEmulator, 1.0);
public:
  HAEmulator();
  ~HAEmulator();
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;

  static constexpr int N_HA = 18;
  unsigned EventID() const { return eventID_; }
  const unsigned short* ADC(int haIndex) const { return &adc_[haIndex * 24]; }   // HA index 0..17 (HA_common ID)
  int HAName(int haIndex) const { return ids_[haIndex]; }

private:
  int LeakPxDef(int pxID, int* ar) const;
  int ADCCh(int ha, int pmt, int* offset) const;

  const EdepPickUp* edep_ = nullptr;
  std::string chargeDivFile_, pmtMapDir_, resolutionFile_, calOutputDir_;
  int randomSeed_;
  double leakPlus_ = .15328, leakX_ = .03233, leakPlusSigma_ = .0265, leakXSigma_ = .00951;

  std::unique_ptr<TRandom3> rndm_;
  double ratio_[64][4] = {};
  std::vector<std::vector<double>> map_;   // [hapmt][64]
  double peak_[N_HA][6][64] = {};
  int ids_[N_HA] = {1, 2, 3, 7, 8, 9, 13, 14, 15, 19, 20, 21, 25, 26, 27, 31, 32, 33};   // SMILE_2p HA_IDs

  unsigned eventID_ = 0;
  std::vector<double> deposit_;
  std::vector<unsigned short> adc_;
};

} // namespace ericasim

#endif
