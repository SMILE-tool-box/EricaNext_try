#ifndef EricaSim_StepLimit_H
#define EricaSim_StepLimit_H 1

#include <string>
#include <vector>
#include <anlnext/BasicModule.hh>

namespace ericasim {

// Sets G4UserLimits(max_step) on the named logical volumes after the geometry
// has been built (chain it after Geant4Body). GDML does not store user
// limits, so a geometry read with ReadGDML loses the 40 um maximum step that
// SMILE_2p_Geo puts on "TPC_detectable"; this module restores it.
class StepLimit : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(StepLimit, 1.0);
public:
  StepLimit();
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
private:
  std::vector<std::string> volumes_;
  double maxStep_;
};

} // namespace ericasim

#endif
