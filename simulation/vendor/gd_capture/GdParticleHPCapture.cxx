// Port of haboc GdParticleHPCapture.cxx to Geant4 11 (see GdParticleHPCapture.h).
// ApplyYourself is the same as the reference and as Geant4 11.4 G4NeutronHPCapture.
#include "GdParticleHPCapture.h"

#include "G4ParticleHPManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4NeutronHPCaptureFS.hh"
#include "G4ParticleHPThermalBoost.hh"
#include "G4Element.hh"
#include "G4Material.hh"
#include "G4Threading.hh"
#include "G4EnvironmentUtils.hh"
#include "G4HadronicException.hh"

#include "GdParticleHPCaptureFS.h"

std::vector<G4ParticleHPChannel*>* GdParticleHPCapture::theCapture = nullptr;
G4int GdParticleHPCapture::numEle = 0;

GdParticleHPCapture::GdParticleHPCapture()
  : G4HadronicInteraction("GdNeutronHPCapture")
{
  SetMinEnergy(0.0);
  SetMaxEnergy(20. * MeV);
}

GdParticleHPCapture::~GdParticleHPCapture()
{
  if (!G4Threading::IsWorkerThread() && theCapture != nullptr) {
    for (auto& ite : *theCapture) delete ite;
    delete theCapture;
    theCapture = nullptr;
    numEle = 0;
  }
}

G4HadFinalState* GdParticleHPCapture::ApplyYourself(const G4HadProjectile& aTrack, G4Nucleus& aNucleus)
{
  G4ParticleHPManager::GetInstance()->OpenReactionWhiteBoard();
  const G4Material* theMaterial = aTrack.GetMaterial();
  auto n = (G4int)theMaterial->GetNumberOfElements();
  std::size_t index = theMaterial->GetElement(0)->GetIndex();
  if (n != 1) {
    auto xSec = new G4double[n];
    G4double sum = 0;
    G4int i;
    const G4double* NumAtomsPerVolume = theMaterial->GetVecNbOfAtomsPerVolume();
    G4double rWeight;
    G4ParticleHPThermalBoost aThermalE;
    for (i = 0; i < n; ++i) {
      index = theMaterial->GetElement(i)->GetIndex();
      rWeight = NumAtomsPerVolume[i];
      xSec[i] = ((*theCapture)[index])->GetXsec(
        aThermalE.GetThermalEnergy(aTrack, theMaterial->GetElement(i), theMaterial->GetTemperature()));
      xSec[i] *= rWeight;
      sum += xSec[i];
    }
    G4double random = G4UniformRand();
    G4double running = 0;
    for (i = 0; i < n; ++i) {
      running += xSec[i];
      index = theMaterial->GetElement(i)->GetIndex();
      if (sum == 0 || random <= running / sum) break;
    }
    delete[] xSec;
  }

  G4HadFinalState* result = ((*theCapture)[index])->ApplyYourself(aTrack);

  // Overwrite target parameters
  G4ParticleHPReactionWhiteBoard* board = G4ParticleHPManager::GetInstance()->GetReactionWhiteBoard();
  aNucleus.SetParameters(board->GetTargA(), board->GetTargZ());
  const G4Element* target_element = (*G4Element::GetElementTable())[index];
  const G4Isotope* target_isotope = nullptr;
  auto iele = (G4int)target_element->GetNumberOfIsotopes();
  for (G4int j = 0; j != iele; ++j) {
    target_isotope = target_element->GetIsotope(j);
    if (target_isotope->GetN() == board->GetTargA()) break;
  }
  aNucleus.SetIsotope(target_isotope);
  G4ParticleHPManager::GetInstance()->CloseReactionWhiteBoard();
  return result;
}

const std::pair<G4double, G4double> GdParticleHPCapture::GetFatalEnergyCheckLevels() const
{
  // reference: (10 %, DBL_MAX)
  return std::pair<G4double, G4double>(10 * perCent, DBL_MAX);
}

void GdParticleHPCapture::BuildPhysicsTable(const G4ParticleDefinition&)
{
  if (!G4Threading::IsMasterThread()) return;   // workers use the master's table
  if (theCapture == nullptr) theCapture = new std::vector<G4ParticleHPChannel*>;
  if (numEle == (G4int)G4Element::GetNumberOfElements()) return;

  if (G4FindDataDir("G4NEUTRONHPDATA") == nullptr)
    throw G4HadronicException(__FILE__, __LINE__,
                              "Please setenv G4NEUTRONHPDATA to point to the neutron cross-section files.");
  G4String dirName = G4String(G4FindDataDir("G4NEUTRONHPDATA")) + "/Capture";

  auto theFS = new G4NeutronHPCaptureFS;
  auto theGdFS = new GdParticleHPCaptureFS;
  for (G4int i = numEle; i < (G4int)G4Element::GetNumberOfElements(); ++i) {
    G4Element* element = (*(G4Element::GetElementTable()))[i];
    theCapture->push_back(new G4ParticleHPChannel);
    ((*theCapture)[i])->Init(element, dirName);
    if (element->GetZasInt() != 64) {
      ((*theCapture)[i])->Register(theFS);
    }
    else {
      G4cout << "GdParticleHPCapture: Gd capture gammas from GdCaptureGammas_ggarnet for "
             << element->GetName() << G4endl;
      ((*theCapture)[i])->Register(theGdFS);
    }
  }
  delete theFS;
  delete theGdFS;
  numEle = (G4int)G4Element::GetNumberOfElements();
}

void GdParticleHPCapture::ModelDescription(std::ostream& outFile) const
{
  outFile << "High Precision model based on Evaluated Nuclear Data Files (ENDF) for radiative capture"
          << " of neutrons below 20 MeV, with the Gd capture gammas of GdCaptureGammas_ggarnet";
}
