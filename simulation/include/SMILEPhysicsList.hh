#ifndef EricaSim_SMILEPhysicsList_H
#define EricaSim_SMILEPhysicsList_H 1

#include <G4VModularPhysicsList.hh>
#include "VANLPhysicsList.hh"

namespace ericasim {

// Port of haboc SMILE_PhysicsList (SMILE_tool_box_v2.20/haboc/src/haboc_g4):
// Livermore EM, decay + radioactive decay, HP hadron elastic, FTFP_BERT_HP,
// stopping, ions, step limiter; production cuts 1 um for gamma/e-/e+ with
// the cut table energy range starting at 250 eV.
// (The Gd neutron-capture variant SMILE_Gd_PhysicsList is not ported yet:
// its capture process uses G4HadronCaptureProcess, removed in Geant4 11.)
class SMILEG4PhysicsList : public G4VModularPhysicsList
{
public:
  SMILEG4PhysicsList(double cut_value, double low_energy_limit, int verbose);
  void SetCuts() override;
private:
  double cutValue_;
  double lowEnergyLimit_;
  int verbose_;
};

// ANLNext module handing the physics list to anlgeant4's Geant4Body.
class SMILEPhysicsList : public anlgeant4::VANLPhysicsList
{
  DEFINE_ANL_MODULE(SMILEPhysicsList, 1.0);
public:
  SMILEPhysicsList();
  anlnext::ANLStatus mod_define() override;
  G4VUserPhysicsList* create() override;
private:
  double cutValue_;          // [mm]
  double lowEnergyLimit_;    // [keV]
  int verbose_;
};

} // namespace ericasim

#endif
