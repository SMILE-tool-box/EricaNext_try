#include "SimTrigger.hh"
#include "EdepPickUp.hh"

#include <iostream>

using namespace anlnext;

namespace ericasim {

SimTrigger::SimTrigger() : daqMode_(3), collectNumber_(0) {}

ANLStatus SimTrigger::mod_define()
{
  define_parameter("daq_mode", &mod_class::daqMode_);
  set_parameter_description("1 TPC, 2 scintillator, 3 TPC && scinti (ETCC), 4 ETCC && no anti, 5 TPC only, 6 scinti only, 0 all");
  define_parameter("collect_number", &mod_class::collectNumber_);
  set_parameter_description("stop when this many events are collected (0: no limit; SMILE_tool_box Collect_Number)");
  return AS_OK;
}

ANLStatus SimTrigger::mod_initialize()
{
  get_module("EdepPickUp", &edep_);
  return AS_OK;
}

ANLStatus SimTrigger::mod_analyze()
{
  // SMILE_2p: the run ends at the start of the event after the last collected one
  if (collectNumber_ > 0 && collected_ >= collectNumber_) return AS_QUIT;
  processed_++;
  const bool tpc = edep_->TPCStepE().size() > 0;
  const bool psa = edep_->ScintiStepE().size() > 0;
  const bool anti = edep_->AntiEnergy() > 0.;
  bool ok = false;
  switch (daqMode_) {     // SMILE_2p::mod_analyze
    case 0: ok = true; break;
    case 1: ok = tpc; break;
    case 2: ok = psa; break;
    case 3: ok = psa && tpc; break;
    case 4: ok = !anti && psa && tpc; break;
    case 5: ok = !psa && tpc; break;
    case 6: ok = psa && !tpc; break;
    default: ok = false;
  }
  if (!ok) return AS_SKIP;
  collected_++;
  return AS_OK;
}

ANLStatus SimTrigger::mod_finalize()
{
  std::cout << "SimTrigger: daq_mode " << daqMode_ << ", collected " << collected_
            << " of " << processed_ << " events" << std::endl;
  return AS_OK;
}

} // namespace ericasim
