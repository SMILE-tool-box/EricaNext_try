#include "GeometryImage.hh"

#include <iostream>
#include <sstream>
#include <G4LogicalVolumeStore.hh>
#include <G4VisAttributes.hh>
#include <G4UImanager.hh>
#include <G4VisExecutive.hh>

using namespace anlnext;

namespace ericasim {

GeometryImage::GeometryImage() = default;
GeometryImage::~GeometryImage() = default;

ANLStatus GeometryImage::mod_define()
{
  define_parameter("commands", &mod_class::commands_);
  set_parameter_description("Geant4 visualization commands applied in order (/vis/open TSG_OFFSCREEN ..., /vis/drawVolume, ...)");
  define_parameter("colours", &mod_class::colours_);
  set_parameter_description("\"<part of the logical-volume name> r g b [alpha]\" (0-1), applied before the commands; later entries win");
  define_parameter("hide", &mod_class::hide_);
  set_parameter_description("parts of logical-volume names not to draw");
  return AS_OK;
}

ANLStatus GeometryImage::mod_initialize()
{
  // colours and visibility by a part of the logical-volume name
  for (G4LogicalVolume* lv : *G4LogicalVolumeStore::GetInstance()) {
    const std::string& name = lv->GetName();
    G4VisAttributes att(lv->GetVisAttributes() ? *lv->GetVisAttributes() : G4VisAttributes());
    bool changed = false;
    for (const std::string& entry : colours_) {
      std::istringstream in(entry);
      std::string key;
      double r = 0, g = 0, b = 0, a = 1;
      in >> key >> r >> g >> b;
      if (!(in >> a)) a = 1;
      if (name.find(key) != std::string::npos) { att.SetColour(r, g, b, a); changed = true; }
    }
    for (const std::string& key : hide_)
      if (name.find(key) != std::string::npos) { att.SetVisibility(false); changed = true; }
    if (changed) lv->SetVisAttributes(att);
  }

  visManager_ = std::make_unique<G4VisExecutive>("errors");
  visManager_->Initialize();
  G4UImanager* ui = G4UImanager::GetUIpointer();
  for (const std::string& cmd : commands_) {
    std::cout << "GeometryImage: " << cmd << std::endl;
    if (ui->ApplyCommand(cmd) != 0) {
      std::cerr << "GeometryImage: command failed: " << cmd << std::endl;
      return AS_QUIT_ERROR;
    }
  }
  return AS_OK;
}

ANLStatus GeometryImage::mod_finalize()
{
  visManager_.reset();
  return AS_OK;
}

} // namespace ericasim
