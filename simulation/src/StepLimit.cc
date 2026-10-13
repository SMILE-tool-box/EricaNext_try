#include "StepLimit.hh"

#include <G4LogicalVolume.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4UserLimits.hh>
#include <G4SystemOfUnits.hh>

namespace ericasim {

StepLimit::StepLimit()
  : volumes_{"TPC_detectable"}, maxStep_(0.040*mm)
{
}

anlnext::ANLStatus StepLimit::mod_define()
{
  define_parameter("volumes", &mod_class::volumes_);
  set_parameter_description("logical volume names");
  define_parameter("max_step", &mod_class::maxStep_, mm, "mm");
  set_parameter_description("maximum step length (SMILE_2p_Geo: 40 um in TPC_detectable)");
  return anlnext::AS_OK;
}

anlnext::ANLStatus StepLimit::mod_initialize()
{
  G4LogicalVolumeStore* store = G4LogicalVolumeStore::GetInstance();
  for (const std::string& name : volumes_) {
    G4LogicalVolume* lv = store->GetVolume(name, false);
    if (!lv) {
      G4cerr << "StepLimit: logical volume " << name << " not found" << G4endl;
      return anlnext::AS_QUIT_ERROR;
    }
    lv->SetUserLimits(new G4UserLimits(maxStep_));
    G4cout << "StepLimit: max step " << maxStep_/mm << " mm in " << name << G4endl;
  }
  return anlnext::AS_OK;
}

} // namespace ericasim
