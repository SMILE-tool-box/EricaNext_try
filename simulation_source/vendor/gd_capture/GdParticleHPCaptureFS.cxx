// Port of haboc GdParticleHPCaptureFS.cxx to Geant4 11 (see GdParticleHPCaptureFS.h).
// The commented-out ENDF / photon-evaporation branches of the reference are not copied.
#include "GdParticleHPCaptureFS.h"

#include <sstream>

#include "G4ParticleHPManager.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4Gamma.hh"
#include "G4Neutron.hh"
#include "G4Proton.hh"
#include "G4ReactionProduct.hh"
#include "G4Nucleus.hh"
#include "G4NucleiProperties.hh"
#include "G4IonTable.hh"
#include "G4ParticleHPDataUsed.hh"

G4HadFinalState* GdParticleHPCaptureFS::ApplyYourself(const G4HadProjectile& theTrack)
{
  if (theResult.Get() == nullptr) theResult.Put(new G4HadFinalState);
  theResult.Get()->Clear();

  // prepare neutron
  G4double eKinetic = theTrack.GetKineticEnergy();
  G4ReactionProduct theNeutron(const_cast<G4ParticleDefinition*>(theTrack.GetDefinition()));
  theNeutron.SetMomentum(theTrack.Get4Momentum().vect());
  theNeutron.SetKineticEnergy(eKinetic);

  // prepare target
  G4Nucleus aNucleus;
  // reference: targetMass (in neutron masses) is always below 500 MeV, so it is always
  // recomputed from the nuclear mass (kept local here: the final state is shared by threads)
  G4double mass = targetMass;
  if (mass < 500 * MeV)
    mass = G4NucleiProperties::GetNuclearMass(theBaseA, theBaseZ) / G4Neutron::Neutron()->GetPDGMass();
  G4ThreeVector neutronVelocity = 1. / G4Neutron::Neutron()->GetPDGMass() * theNeutron.GetMomentum();
  G4double temperature = theTrack.GetMaterial()->GetTemperature();
  G4ReactionProduct theTarget = aNucleus.GetBiasedThermalNucleus(mass, neutronVelocity, temperature);
  theTarget.SetDefinition(G4IonTable::GetIonTable()->GetIon(theBaseZ, theBaseA, 0.0));

  // go to nucleus rest system
  theNeutron.Lorentz(theNeutron, -1 * theTarget);

  // dice the photons (Gd model instead of the ENDF data)
  G4ReactionProductVector* thePhotons = theFinalGammas.GetGammas();

  // update the nucleus
  G4ThreeVector aCMSMomentum = theNeutron.GetMomentum() + theTarget.GetMomentum();
  G4LorentzVector p4(aCMSMomentum, theTarget.GetTotalEnergy() + theNeutron.GetTotalEnergy());
  G4Fragment nucleus(theBaseA + 1, theBaseZ, p4);
  const G4int nPhotons = (thePhotons != nullptr) ? (G4int)thePhotons->size() : 0;
  for (G4int i = 0; i < nPhotons; ++i) {
    G4ThreeVector pGamma((*thePhotons)[i]->GetMomentum());
    G4LorentzVector gamma(pGamma, (*thePhotons)[i]->GetTotalEnergy());
    G4Fragment theOne(gamma, G4Gamma::GammaDefinition());
    UpdateNucleus(&nucleus, &theOne, (*thePhotons)[i]->GetTotalEnergy());
  }
  auto theTwo = new G4DynamicParticle;
  theTwo->SetDefinition(G4IonTable::GetIonTable()->GetIon(theBaseZ, theBaseA + 1, 0));
  theTwo->SetMomentum(nucleus.GetMomentum());

  // back to lab system
  for (G4int i = 0; i < nPhotons; ++i) {
    (*thePhotons)[i]->Lorentz(*((*thePhotons)[i]), theTarget);
  }

  // Recoil, if only one gamma (reference: added in addition to the residual below)
  if (nPhotons == 1 && (*thePhotons)[0]->GetDefinition()->GetBaryonNumber() == 0) {
    auto theOne = new G4DynamicParticle;
    theOne->SetDefinition(G4IonTable::GetIonTable()->GetIon(theBaseZ, theBaseA + 1, 0));
    G4ThreeVector aMomentum = theTrack.Get4Momentum().vect() + theTarget.GetMomentum()
                              - (*thePhotons)[0]->GetMomentum();
    theOne->SetMomentum(aMomentum);
    theResult.Get()->AddSecondary(theOne, secID);
  }

  // Now fill in the gammas.
  for (G4int i = 0; i < nPhotons; ++i) {
    auto theOne = new G4DynamicParticle;
    theOne->SetDefinition((*thePhotons)[i]->GetDefinition());
    theOne->SetMomentum((*thePhotons)[i]->GetMomentum());
    theResult.Get()->AddSecondary(theOne, secID);
    delete (*thePhotons)[i];
  }
  delete thePhotons;

  // de-excited nucleus
  theResult.Get()->AddSecondary(theTwo, secID);

  // clean up the primary neutron
  theResult.Get()->SetStatusChange(stopAndKill);
  return theResult.Get();
}

void GdParticleHPCaptureFS::UpdateNucleus(G4Fragment* nucleus, const G4Fragment* gamma, G4double eGamma)
{
  G4LorentzVector p4Gamma = gamma->GetMomentum();
  G4ThreeVector pGamma(p4Gamma.vect());
  G4LorentzVector p4Nucleus(nucleus->GetMomentum());

  G4IonTable* theTable = G4IonTable::GetIonTable();
  G4double mass1 = (theTable->GetIon(nucleus->GetZ_asInt(), nucleus->GetA_asInt(), 0))->GetPDGMass();
  G4double mass2 = nucleus->GetZ_asInt() * G4Proton::Proton()->GetPDGMass()
                   + (nucleus->GetA_asInt() - nucleus->GetZ_asInt()) * G4Neutron::Neutron()->GetPDGMass();
  G4double Mass = std::min(mass1, mass2);

  G4double newExcitation = p4Nucleus.mag() - Mass - eGamma;
  if (newExcitation < 0) newExcitation = 0;

  G4ThreeVector p3Residual(p4Nucleus.vect() - pGamma);
  G4double newEnergy = std::sqrt(p3Residual * p3Residual + (Mass + newExcitation) * (Mass + newExcitation));
  G4LorentzVector p4Residual(p3Residual, newEnergy);

  // Update excited nucleus parameters
  nucleus->SetMomentum(p4Residual);
}

void GdParticleHPCaptureFS::Init(G4double AA, G4double ZZ, G4int M, const G4String& dirName,
                                 const G4String&, G4ParticleDefinition*)
{
  // same as the reference (and the Geant4 11 G4NeutronHPCaptureFS::Init); only
  // targetMass is used by ApplyYourself
  G4int Z = G4lrint(ZZ);
  G4int A = G4lrint(AA);
  std::stringstream ss;
  ss << Z;
  G4String sZ;
  ss >> sZ;
  ss.clear();
  ss << A;
  G4String sA;
  ss >> sA;
  ss.clear();
  G4String sM;
  if (M > 0) {
    ss << "m";
    ss << M;
    ss >> sM;
    ss.clear();
  }
  G4String element_name = theNames.GetName(Z - 1);
  G4String filenameMF6 = dirName + "/FSMF6/" + sZ + "_" + sA + sM + "_" + element_name;
  std::istringstream theData(std::ios::in);
  G4ParticleHPManager::GetInstance()->GetDataStream(filenameMF6, theData);
  if (theData.good()) {
    hasExactMF6 = true;
    theMF6FinalState.Init(theData);
    return;
  }

  const G4String tString = "/FS";
  G4bool dbool;
  const G4ParticleHPDataUsed& aFile = theNames.GetName(A, Z, M, dirName, tString, dbool);
  const G4String& filename = aFile.GetName();
  SetAZMs(A, Z, M, aFile);
  if (!dbool || (Z <= 2 && (theBaseZ != Z || theBaseA != A))) {
    hasAnyData = false;
    hasFSData = false;
    hasXsec = false;
    return;
  }
  theData.clear();
  G4ParticleHPManager::GetInstance()->GetDataStream(filename, theData);
  hasFSData = theFinalStatePhotons.InitMean(theData);
  if (hasFSData) {
    targetMass = theFinalStatePhotons.GetTargetMass();
    theFinalStatePhotons.InitAngular(theData);
    theFinalStatePhotons.InitEnergies(theData);
  }
}
