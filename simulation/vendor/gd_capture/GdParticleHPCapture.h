// Port of haboc GdParticleHPCapture (SMILE_tool_box_v2.20/haboc) to Geant4 11.
// Neutron capture below 20 MeV with the ENDF data (as G4NeutronHPCapture), except that the
// final state of Gd (Z = 64) is GdParticleHPCaptureFS.
// Unlike the reference, the channel table is not the one shared through G4ParticleHPManager
// (RegisterCaptureFinalStates): if the standard capture model built that table first, the
// Gd final state would silently not be used. The table is built on the master thread and
// read by the workers.
#ifndef GdParticleHPCapture_h
#define GdParticleHPCapture_h 1

#include <vector>
#include "globals.hh"
#include "G4HadronicInteraction.hh"
#include "G4ParticleHPChannel.hh"

class GdParticleHPCapture : public G4HadronicInteraction
{
public:
  GdParticleHPCapture();
  ~GdParticleHPCapture() override;

  G4HadFinalState* ApplyYourself(const G4HadProjectile& aTrack, G4Nucleus& aTargetNucleus) override;
  const std::pair<G4double, G4double> GetFatalEnergyCheckLevels() const override;
  void BuildPhysicsTable(const G4ParticleDefinition&) override;
  void ModelDescription(std::ostream& outFile) const override;

private:
  static std::vector<G4ParticleHPChannel*>* theCapture;
  static G4int numEle;
};

#endif
