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
#include <G4Exception.hh>

#include "GdNeutronCapturePhysics.h"
#include "SMILEEmLivermoreUrbanPhysics.hh"
#include "GdCaptureGammas_ggarnet.h"

namespace ericasim {

SMILEG4PhysicsList::SMILEG4PhysicsList(double cut_value, double low_energy_limit, bool gd_physics,
                                       bool urban_msc, int verbose)
  : cutValue_(cut_value), lowEnergyLimit_(low_energy_limit), verbose_(verbose)
{
  const int ver = verbose;
  if (urban_msc) RegisterPhysics(new SMILEEmLivermoreUrbanPhysics(ver));
  else RegisterPhysics(new G4EmLivermorePhysics(ver));
  RegisterPhysics(new G4DecayPhysics(ver));
  RegisterPhysics(new G4RadioactiveDecayPhysics(ver));
  RegisterPhysics(new G4HadronElasticPhysicsHP(ver));
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP(ver));
  // haboc SMILE_Gd_PhysicsList: right after the hadron physics, as there (replaces nCapture)
  if (gd_physics) RegisterPhysics(new GdNeutronCapturePhysics(ver));
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
  : cutValue_(0.001 * mm), lowEnergyLimit_(0.25 * keV),   // haboc: 1 um, 250 eV
    gdPhysics_(false), emMsc_("gs"), verbose_(0)
{
}

anlnext::ANLStatus SMILEPhysicsList::mod_define()
{
  define_parameter("cut_value", &mod_class::cutValue_, mm, "mm");
  set_parameter_description("production cut for gamma, e-, e+ (haboc: 1 um)");
  define_parameter("low_energy_limit", &mod_class::lowEnergyLimit_, keV, "keV");
  set_parameter_description("lower edge of the production-cut energy table (haboc: 250 eV)");
  define_parameter("gd_physics", &mod_class::gdPhysics_);
  set_parameter_description("Gd neutron capture model (haboc macro GdPhysics 1)");
  define_parameter("em_msc", &mod_class::emMsc_);
  set_parameter_description("e+- multiple scattering: gs (Geant4 11 G4EmLivermorePhysics) or urban (reference, Geant4 10.4)");
  define_parameter("gd155_table", &mod_class::gd155Table_);
  set_parameter_description("continuum table of the Gd capture model for Gd-155 (Gd155.dat)");
  define_parameter("gd157_table", &mod_class::gd157Table_);
  set_parameter_description("continuum table of the Gd capture model for Gd-157 (Gd157_org.dat)");
  define_parameter("verbose", &mod_class::verbose_);
  return anlnext::AS_OK;
}

G4VUserPhysicsList* SMILEPhysicsList::create()
{
  if (gdPhysics_) {
    if (gd155Table_.empty() || gd157Table_.empty()) {
      G4Exception("SMILEPhysicsList::create", "EricaSim001", FatalException,
                  "gd_physics needs gd155_table and gd157_table");
    }
    GdCaptureGammas_ggarnet::SetDataFiles(gd155Table_, gd157Table_);
  }
  if (emMsc_ != "gs" && emMsc_ != "urban") {
    G4Exception("SMILEPhysicsList::create", "EricaSim002", FatalException, "em_msc must be gs or urban");
  }
  return new SMILEG4PhysicsList(cutValue_, lowEnergyLimit_, gdPhysics_, emMsc_ == "urban", verbose_);
}

} // namespace ericasim
