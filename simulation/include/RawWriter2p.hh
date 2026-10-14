#ifndef EricaSim_RawWriter2p_H
#define EricaSim_RawWriter2p_H 1

#include <memory>
#include <string>
#include <anlnext/BasicModule.hh>

class TFile;
class TTree;
class TPC_FileManager;
class HA_FileManager;

namespace ericasim {

class EdepPickUp;
class TPCEmulator;
class HAEmulator;

// Port of SMILE_tool_box_v2.20 EmuWrite2p (Erica/EmuWrite2p) to ANLNext: writes the output of
// TPCEmulator / HAEmulator as raw files of the SMILE-2+ DAQ (vendor/emu_write TPC_FileManager,
// HA_FileManager, unchanged) into output_dir:
//   uTPC_anode<b>_NNNNN.raw, uTPC_cathode<b>_NNNNN.raw (b = 1..3), HA<name>_NNNNN.raw (18 HAs)
// which ReadData reads like measured data. Event ID in the headers = Geant4 event ID (low 24 bits).
// truth_file (optional): TTree "sim" with the event ID, the primary and the energy deposits of
// every written event (EmuWrite2p's sim tree: EventID, SrcEng, SrcPos, SrcDir, E_psa, E_tpc, E_anti).
class RawWriter2p : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(RawWriter2p, 1.0);
public:
  RawWriter2p();
  ~RawWriter2p();
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_finalize() override;

private:
  std::string outputDir_, truthFile_;
  bool writeHA_, writeAnode_, writeCathode_;

  const EdepPickUp* edep_ = nullptr;
  const TPCEmulator* tpc_ = nullptr;
  const HAEmulator* ha_ = nullptr;
  std::unique_ptr<TPC_FileManager> tpcFM_[2][16];
  std::unique_ptr<HA_FileManager> haFM_[18];

  TFile* truth_ = nullptr;
  TTree* tree_ = nullptr;
  int evid_ = 0;
  double srcEng_ = 0, srcPos_[3] = {}, srcDir_[3] = {}, ePSA_ = 0, eTPC_ = 0, eAnti_ = 0;
  long nWritten_ = 0;
};

} // namespace ericasim

#endif
