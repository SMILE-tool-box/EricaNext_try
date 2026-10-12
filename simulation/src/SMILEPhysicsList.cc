#include "SMILEPhysicsList.hh"

#include <G4SystemOfUnits.hh>
#include <G4UnitsTable.hh>
#include <G4ProductionCutsTable.hh>
#include <G4EmLivermorePhysics.hh>
#include <G4DecayPhysics.hh>
#include <G4RadioactiveDecayPhysics.hh>
#include <G4HadronElasticPhysicsHP.hh>
#include <G4HadronPhysicsFTFP_BERT_HP.hh>
#include <G4StoppingPhysics.hh>
#include <G4IonPhysics.hh>
#include <G4StepLimiterPhysics.hh>

namespace ericasim {

SMILEG4PhysicsList::SMILEG4PhysicsList(double cut_value, double low_energy_limit, int verbose)
  : cutValue_(cut_value), lowEnergyLimit_(low_energy_limit), verbose_(verbose)
{
  const int ver = verbose;
  RegisterPhysics(new G4EmLivermorePhysics(ver));
  RegisterPhysics(new G4DecayPhysics(ver));
  RegisterPhysics(new G4RadioactiveDecayPhysics(ver));
  RegisterPhysics(new G4HadronElasticPhysicsHP(ver));
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP(ver));
  RegisterPhysics(new G4StoppingPhysics(ver));
  RegisterPhysics(new G4IonPhysics(ver));
  RegisterPhysics(new G4StepLimiterPhysics());   // Geant4 11: no verbose argument
}

void SMILEG4PhysicsList::SetCuts()
{
  // haboc SMILE_PhysicsList::SetCuts(): energy range from 250 eV, 1 um cuts
  G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange(lowEnergyLimit_, 100.*GeV);
  SetCutValue(cutValue_, "gamma");
  SetCutValue(cutValue_, "e-");
  SetCutValue(cutValue_, "e+");
  if (verbose_ > 0) DumpCutValuesTable();
}

SMILEPhysicsList::SMILEPhysicsList()
  : cutValue_(0.001), lowEnergyLimit_(0.25), verbose_(0)
{
}

anlnext::ANLStatus SMILEPhysicsList::mod_define()
{
  define_parameter("cut_value", &mod_class::cutValue_, mm, "mm");
  set_parameter_description("production cut for gamma, e-, e+ (haboc: 1 um)");
  define_parameter("low_energy_limit", &mod_class::lowEnergyLimit_, keV, "keV");
  set_parameter_description("lower edge of the production-cut energy table (haboc: 250 eV)");
  define_parameter("verbose", &mod_class::verbose_);
  return anlnext::AS_OK;
}

G4VUserPhysicsList* SMILEPhysicsList::create()
{
  return new SMILEG4PhysicsList(cutValue_, lowEnergyLimit_, verbose_);
}

} // namespace ericasim
