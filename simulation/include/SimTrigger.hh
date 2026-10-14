#ifndef EricaSim_SimTrigger_H
#define EricaSim_SimTrigger_H 1

#include <anlnext/BasicModule.hh>

namespace ericasim {

class EdepPickUp;

// Event selection of SMILE_2p (SMILE_tool_box, DAQ_Mode / Collect_Number). Put it after
// EdepPickUp: events that do not pass are skipped, so the detector-response modules
// (TPCEmulator, HAEmulator, RawWriter2p) after it only see collected events.
//   daq_mode 1: TPC, 2: scintillator, 3: TPC && scintillator (ETCC),
//            4: TPC && scintillator && no anti, 5: TPC only, 6: scintillator only, 0: all events
// "TPC" / "scintillator" / "anti" = any energy deposit there (scintillator after HA_OFF).
// collect_number > 0: stop the run when that many events are collected.
class SimTrigger : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(SimTrigger, 1.0);
public:
  SimTrigger();
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_finalize() override;

  long Collected() const { return collected_; }

private:
  const EdepPickUp* edep_ = nullptr;
  int daqMode_;
  long collectNumber_;
  long collected_ = 0;
  long processed_ = 0;
};

} // namespace ericasim

#endif
