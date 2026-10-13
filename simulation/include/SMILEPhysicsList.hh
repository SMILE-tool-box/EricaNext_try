#ifndef EricaSim_SMILEPhysicsList_H
#define EricaSim_SMILEPhysicsList_H 1

#include <string>
#include <G4VModularPhysicsList.hh>
#include "VANLPhysicsList.hh"

namespace ericasim {

// Port of haboc SMILE_PhysicsList (SMILE_tool_box_v2.20/haboc/src/haboc_g4):
// Livermore EM, decay + radioactive decay, HP hadron elastic, FTFP_BERT_HP,
// stopping, ions, step limiter; production cuts 1 um for gamma/e-/e+ with
// the cut table energy range starting at 250 eV.
// gd_physics = true gives SMILE_Gd_PhysicsList (macro "GdPhysics 1"): the same list plus
// GdNeutronCapturePhysics (vendor/gd_capture, port of SKNeutronCapturePhysics).
// em_msc = "urban": SMILEEmLivermoreUrbanPhysics (e+- msc as in the reference, Geant4 10.4).
class SMILEG4PhysicsList : public G4VModularPhysicsList
{
public:
  SMILEG4PhysicsList(double cut_value, double low_energy_limit, bool gd_physics,
                     bool urban_msc, int verbose);
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
  bool gdPhysics_;
  std::string emMsc_;        // "gs" (Geant4 11 G4EmLivermorePhysics) or "urban" (reference 10.4 e+- msc)
  std::string gd155Table_;
  std::string gd157Table_;
  int verbose_;
};

} // namespace ericasim

#endif
