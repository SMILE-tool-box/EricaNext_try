// Port of haboc SKNeutronCapturePhysics.cxx to Geant4 11 (see GdNeutronCapturePhysics.h).
#include "GdNeutronCapturePhysics.h"

#include "G4SystemOfUnits.hh"
#include "G4PhysicsListHelper.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessTable.hh"
#include "G4Neutron.hh"
#include "G4NeutronCaptureProcess.hh"
#include "G4NeutronRadCapture.hh"
#include "G4NeutronCaptureXS.hh"
#include "G4ParticleHPCaptureData.hh"
#include "G4CrossSectionDataSetRegistry.hh"

#include "GdParticleHPCapture.h"

GdNeutronCapturePhysics::GdNeutronCapturePhysics(G4int ver)
  : G4VPhysicsConstructor("GdNeutronCapturePhysics"), verbose(ver)
{
}

void GdNeutronCapturePhysics::ConstructParticle()
{
  G4Neutron::Neutron();
}

void GdNeutronCapturePhysics::ConstructProcess()
{
  G4ParticleDefinition* neutron = G4Neutron::Neutron();
  G4ProcessManager* pManager = neutron->GetProcessManager();

  // To remove the existing nCapture process
  G4VProcess* process = G4ProcessTable::GetProcessTable()->FindProcess("nCapture", neutron);
  if (process != nullptr) {
    if (verbose > 0)
      G4cout << "GdNeutronCapturePhysics::ConstructProcess: Removing existing process for neutron nCapture" << G4endl;
    pManager->RemoveProcess(process);
  }

  auto theCaptureProcess = new G4NeutronCaptureProcess;

  // 0 - 20 MeV --> GdParticleHPCapture (Gd final state by the SK collaboration / Okayama)
  theCaptureProcess->RegisterMe(new GdParticleHPCapture);

  // > 20 MeV --> G4NeutronRadCapture
  auto lcaptureModel = new G4NeutronRadCapture;
  lcaptureModel->SetMinEnergy(20 * MeV);
  theCaptureProcess->RegisterMe(lcaptureModel);

  // as the reference (copied there from G4HadronPhysicsQGSP_BERT_HP)
  auto xs_n_in = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet(G4NeutronCaptureXS::Default_Name());
  if (xs_n_in == nullptr) xs_n_in = new G4NeutronCaptureXS;
  theCaptureProcess->AddDataSet(xs_n_in);
  theCaptureProcess->AddDataSet(new G4ParticleHPCaptureData);

  G4PhysicsListHelper::GetPhysicsListHelper()->RegisterProcess(theCaptureProcess, neutron);
}
