// Port of haboc GdParticleHPCaptureFS (SMILE_tool_box_v2.20/haboc, after the
// Geant4 10 G4ParticleHPCaptureFS, P. Arce / T. Koi; Gd part by Hagiwara) to Geant4 11.
// Final state of neutron capture on Gd: the gammas come from GdCaptureGammas_ggarnet
// (not from the ENDF photon data), and the residual nucleus takes the remaining momentum.
// Changes for Geant4 11 / multi-threading only (no change of the physics):
//  - Init() takes const references (Geant4 11 G4ParticleHPFinalState)
//  - the per-call state (nucleus, residual) is local instead of data members, because
//    the channels and their final states are shared by the worker threads
//  - the temporary G4Fragment objects are deleted
#ifndef GdParticleHPCaptureFS_h
#define GdParticleHPCaptureFS_h 1

#include "GdCaptureGammas_ggarnet.h"

#include "globals.hh"
#include "G4Fragment.hh"
#include "G4HadProjectile.hh"
#include "G4HadFinalState.hh"
#include "G4ParticleHPFinalState.hh"
#include "G4ReactionProductVector.hh"
#include "G4ParticleHPNames.hh"
#include "G4ParticleHPPhotonDist.hh"
#include "G4ParticleHPEnAngCorrelation.hh"

class GdParticleHPCaptureFS : public G4ParticleHPFinalState
{
public:
  GdParticleHPCaptureFS()
  {
    hasXsec = false;
  }
  ~GdParticleHPCaptureFS() override = default;

  void Init(G4double A, G4double Z, G4int M, const G4String& dirName,
            const G4String& aFSType, G4ParticleDefinition*) override;
  G4HadFinalState* ApplyYourself(const G4HadProjectile& theTrack) override;
  G4ParticleHPFinalState* New() override { return new GdParticleHPCaptureFS; }

private:
  // reference UpdateNucleus(): remove one gamma from the excited nucleus
  static void UpdateNucleus(G4Fragment* nucleus, const G4Fragment* gamma, G4double eGamma);

  G4double targetMass{0.};
  G4bool hasExactMF6{false};
  G4ParticleHPPhotonDist theFinalStatePhotons;
  G4ParticleHPEnAngCorrelation theMF6FinalState;
  G4ParticleHPNames theNames;

  GdCaptureGammas_ggarnet theFinalGammas;
};

#endif
