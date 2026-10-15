#ifndef EricaSim_SMILE2pGeometry_H
#define EricaSim_SMILE2pGeometry_H 1

#include <memory>
#include <string>
#include "VANLGeometry.hh"

class SMILE_Material;
class HAPositionManager;

namespace ericasim {

// SMILE-2+ geometry built exactly as SMILE_tool_box_v2.20 SMILE_2p does:
// SMILE_2p_Geo (vendor/smile2p_geo) assembles the .mqo meshes (vendor/ggg)
// and the HA scintillator pixels placed from the HA position file.
// Parameters correspond to the haboc macro [SMILE_2p] entries.
class SMILE2pGeometry : public anlgeant4::VANLGeometry
{
  DEFINE_ANL_MODULE(SMILE2pGeometry, 1.0);
public:
  SMILE2pGeometry();
  ~SMILE2pGeometry();
  anlnext::ANLStatus mod_define() override;
  G4VUserDetectorConstruction* create() override;
private:
  std::string geometryDir_;      // GeometryCADdir
  std::string geometryType_;     // GeometryType: ground / flight / no_vessel
  std::string haPositionFile_;   // HA_PositionFile
  double gasPressure_;           // TPC_Gas_Pressure [atm]
  std::unique_ptr<SMILE_Material> material_;
  std::unique_ptr<HAPositionManager> haPosition_;
};

} // namespace ericasim

#endif
