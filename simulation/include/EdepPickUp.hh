#ifndef EricaSim_EdepPickUp_H
#define EricaSim_EdepPickUp_H 1

#include <set>
#include <string>
#include <utility>
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
// ha_off_file (haboc HA_OffFile, data/HA_OFF.dat: lines "HA_name PMT(ID+1)"): scintillator
// deposits in those PMTs are dropped, as SMILE_2p::DepositInPSA does.
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

  // the current event, for the detector-response modules after this one in the chain
  int EventID() const { return eventID_; }
  const std::string& PrimaryName() const { return primaryName_; }
  double PrimaryEnergy() const { return primaryEnergy_; }   // keV
  const double* PrimaryPos() const { return primaryPos_; }   // mm
  const double* PrimaryDir() const { return primaryDir_; }
  double TPCEnergy() const { return eTPC_; }        // keV
  double ScintiEnergy() const { return eScinti_; }
  double AntiEnergy() const { return eAnti_; }
  const std::vector<double>& TPCStepE() const { return tpcE_; }   // keV
  const std::vector<double>& TPCStepX() const { return tpcX_; }   // mm (world)
  const std::vector<double>& TPCStepY() const { return tpcY_; }
  const std::vector<double>& TPCStepZ() const { return tpcZ_; }
  const std::vector<std::string>& ScintiStepVolume() const { return sciVolume_; }
  const std::vector<double>& ScintiStepE() const { return sciE_; }
  const std::vector<double>& ScintiStepX() const { return sciX_; }
  const std::vector<double>& ScintiStepY() const { return sciY_; }
  const std::vector<double>& ScintiStepZ() const { return sciZ_; }

private:
  std::string outputFile_;
  std::string tpcVolume_, scintiKeyword_, antiVolume_;
  std::string haOffFile_;
  std::set<std::pair<int, int>> haOff_;   // (HA name, PMT name) switched off
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
  std::vector<int>    tpcPDG_;              // particle of the step
  std::vector<std::string> tpcCreator_;     // creator process of the track ("primary" for the primary)
  std::vector<double> tpcE_, tpcX_, tpcY_, tpcZ_;  // keV, mm
  std::vector<std::string> sciVolume_;
  std::vector<int>    sciCopy_;
  std::vector<double> sciE_, sciX_, sciY_, sciZ_;
};

} // namespace ericasim

#endif
