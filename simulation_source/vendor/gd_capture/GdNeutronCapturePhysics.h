// Port of haboc SKNeutronCapturePhysics (SMILE_tool_box_v2.20/haboc) to Geant4 11:
// replaces the neutron nCapture process by one with GdParticleHPCapture (0-20 MeV)
// and G4NeutronRadCapture (> 20 MeV). Register it after the hadron physics.
// Geant4 11: G4HadronCaptureProcess -> G4NeutronCaptureProcess.
#ifndef GdNeutronCapturePhysics_h
#define GdNeutronCapturePhysics_h 1

#include "globals.hh"
#include "G4VPhysicsConstructor.hh"

class GdNeutronCapturePhysics : public G4VPhysicsConstructor
{
public:
  explicit GdNeutronCapturePhysics(G4int ver = 1);
  ~GdNeutronCapturePhysics() override = default;
  void ConstructParticle() override;
  void ConstructProcess() override;

private:
  G4int verbose;
};

#endif
