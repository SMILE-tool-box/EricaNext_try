#ifndef EricaSim_EdepPickUp_H
#define EricaSim_EdepPickUp_H 1

#include <string>
#include <vector>
#include "VAppendableUserActionAssembly.hh"

class TFile;
class TTree;

namespace ericasim {

// Records, per event, every Geant4 step with an energy deposit in
//   - the TPC gas volume  (name == tpc_volume,   haboc: "TPC_detectable")
//   - scintillator volumes (name contains scinti_keyword, haboc: "scinti")
//   - the anti-coincidence volume (name == anti_volume, haboc: "anti")
// like SMILE_2p_Step::UserSteppingAction (edep with the post-step position
// track->GetPosition()), plus the primary particle, into the TTree "edep".
// This is the input of the detector-response modules (TPC / HA emulators).
class EdepPickUp : public anlgeant4::VAppendableUserActionAssembly
{
  DEFINE_ANL_MODULE(EdepPickUp, 1.0);
public:
  EdepPickUp();
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_finalize() override;

  void EventActionAtBeginning(const G4Event* event) override;
  void EventActionAtEnd(const G4Event* event) override;
  void SteppingAction(const G4Step* step) override;

private:
  std::string outputFile_;
  std::string tpcVolume_, scintiKeyword_, antiVolume_;
  bool keepEmptyEvents_;

  TFile* file_ = nullptr;
  TTree* tree_ = nullptr;

  // per event
  int eventID_ = 0;
  std::string primaryName_;
  double primaryEnergy_ = 0.;              // keV
  double primaryPos_[3] = {0, 0, 0};       // mm
  double primaryDir_[3] = {0, 0, 0};
  double eTPC_ = 0., eScinti_ = 0., eAnti_ = 0.;   // keV
  std::vector<int>    tpcTrack_;
  std::vector<double> tpcE_, tpcX_, tpcY_, tpcZ_;  // keV, mm
  std::vector<std::string> sciVolume_;
  std::vector<int>    sciCopy_;
  std::vector<double> sciE_, sciX_, sciY_, sciZ_;
};

} // namespace ericasim

#endif
