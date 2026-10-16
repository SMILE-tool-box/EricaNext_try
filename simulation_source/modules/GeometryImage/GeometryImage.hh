#ifndef EricaSim_GeometryImage_H
#define EricaSim_GeometryImage_H 1

#include <memory>
#include <string>
#include <vector>
#include <anlnext/BasicModule.hh>

class G4VisManager;

namespace ericasim {

// Draws the Geant4 geometry to an image file without a display: starts the Geant4 visualization
// (G4VisExecutive) and applies the visualization commands given in "commands", e.g.
//   /vis/open TSG_OFFSCREEN 1200x1000, /vis/drawVolume, /vis/viewer/set/style surface,
//   /vis/tsg/offscreen/set/file geometry.png, /vis/viewer/rebuild
// Before the commands, logical volumes can be coloured / hidden by a part of their name ("colours", "hide"),
// since the visualization commands address one logical volume at a time (the HA pixels are ~2.7e4 volumes).
// Chain it after Geant4Body (the geometry is built in Geant4Body's initialization).
class GeometryImage : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(GeometryImage, 1.0);
public:
  GeometryImage();
  ~GeometryImage();

  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_finalize() override;

private:
  std::vector<std::string> commands_;
  std::vector<std::string> colours_;   // "<part of the name> r g b [alpha]" (later entries win)
  std::vector<std::string> hide_;      // parts of the names of volumes not drawn
  std::unique_ptr<G4VisManager> visManager_;
};

} // namespace ericasim

#endif
