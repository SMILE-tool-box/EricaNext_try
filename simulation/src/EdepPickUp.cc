#include "EdepPickUp.hh"

#include <G4Event.hh>
#include <G4PrimaryVertex.hh>
#include <G4PrimaryParticle.hh>
#include <G4ParticleDefinition.hh>
#include <G4Step.hh>
#include <G4Track.hh>
#include <G4VPhysicalVolume.hh>
#include <G4SystemOfUnits.hh>
#include <TFile.h>
#include <TTree.h>

namespace ericasim {

EdepPickUp::EdepPickUp()
  : outputFile_("edep.root"), tpcVolume_("TPC_detectable"),
    scintiKeyword_("scinti"), antiVolume_("anti"), keepEmptyEvents_(false)
{
}

anlnext::ANLStatus EdepPickUp::mod_define()
{
  define_parameter("output", &mod_class::outputFile_);
  define_parameter("tpc_volume", &mod_class::tpcVolume_);
  define_parameter("scinti_keyword", &mod_class::scintiKeyword_);
  define_parameter("anti_volume", &mod_class::antiVolume_);
  define_parameter("keep_empty_events", &mod_class::keepEmptyEvents_);
  set_parameter_description("also write events without any deposit in the TPC / scintillators");
  return anlnext::AS_OK;
}

anlnext::ANLStatus EdepPickUp::mod_initialize()
{
  file_ = new TFile(outputFile_.c_str(), "recreate");
  tree_ = new TTree("edep", "energy deposits per event (keV, mm)");
  tree_->Branch("event_id", &eventID_);
  tree_->Branch("primary_name", &primaryName_);
  tree_->Branch("primary_energy", &primaryEnergy_);
  tree_->Branch("primary_pos", primaryPos_, "primary_pos[3]/D");
  tree_->Branch("primary_dir", primaryDir_, "primary_dir[3]/D");
  tree_->Branch("e_tpc", &eTPC_);
  tree_->Branch("e_scinti", &eScinti_);
  tree_->Branch("e_anti", &eAnti_);
  tree_->Branch("tpc_track", &tpcTrack_);
  tree_->Branch("tpc_e", &tpcE_);
  tree_->Branch("tpc_x", &tpcX_);
  tree_->Branch("tpc_y", &tpcY_);
  tree_->Branch("tpc_z", &tpcZ_);
  tree_->Branch("sci_volume", &sciVolume_);
  tree_->Branch("sci_copy", &sciCopy_);
  tree_->Branch("sci_e", &sciE_);
  tree_->Branch("sci_x", &sciX_);
  tree_->Branch("sci_y", &sciY_);
  tree_->Branch("sci_z", &sciZ_);
  return anlnext::AS_OK;
}

anlnext::ANLStatus EdepPickUp::mod_finalize()
{
  if (file_) {
    file_->cd();
    tree_->Write();
    file_->Close();
    delete file_;
    file_ = nullptr;
  }
  return anlnext::AS_OK;
}

void EdepPickUp::EventActionAtBeginning(const G4Event* event)
{
  eventID_ = event->GetEventID();
  eTPC_ = eScinti_ = eAnti_ = 0.;
  tpcTrack_.clear(); tpcE_.clear(); tpcX_.clear(); tpcY_.clear(); tpcZ_.clear();
  sciVolume_.clear(); sciCopy_.clear(); sciE_.clear(); sciX_.clear(); sciY_.clear(); sciZ_.clear();

  primaryName_ = ""; primaryEnergy_ = 0.;
  const G4PrimaryVertex* vertex = event->GetPrimaryVertex();
  if (vertex && vertex->GetPrimary()) {
    const G4PrimaryParticle* p = vertex->GetPrimary();
    primaryName_ = p->GetParticleDefinition() ? p->GetParticleDefinition()->GetParticleName() : "";
    primaryEnergy_ = p->GetKineticEnergy() / keV;
    const G4ThreeVector pos = vertex->GetPosition() / mm;
    const G4ThreeVector dir = p->GetMomentumDirection();
    for (int k = 0; k < 3; k++) { primaryPos_[k] = pos[k]; primaryDir_[k] = dir[k]; }
  }
}

void EdepPickUp::SteppingAction(const G4Step* step)
{
  const double edep = step->GetTotalEnergyDeposit();
  if (edep <= 0.) return;
  const G4Track* track = step->GetTrack();
  const G4VPhysicalVolume* volume = track->GetVolume();
  if (!volume) return;
  const std::string& name = volume->GetName();
  const G4ThreeVector pos = track->GetPosition() / mm;   // as SMILE_2p_Step
  const double e = edep / keV;

  if (name == tpcVolume_) {
    eTPC_ += e;
    tpcTrack_.push_back(track->GetTrackID());
    tpcE_.push_back(e); tpcX_.push_back(pos.x()); tpcY_.push_back(pos.y()); tpcZ_.push_back(pos.z());
  }
  else if (name.find(scintiKeyword_) != std::string::npos) {
    eScinti_ += e;
    sciVolume_.push_back(name);
    sciCopy_.push_back(step->GetPreStepPoint()->GetTouchable()->GetCopyNumber());
    sciE_.push_back(e); sciX_.push_back(pos.x()); sciY_.push_back(pos.y()); sciZ_.push_back(pos.z());
  }
  else if (name == antiVolume_) {
    eAnti_ += e;
  }
}

void EdepPickUp::EventActionAtEnd(const G4Event*)
{
  if (keepEmptyEvents_ || !tpcE_.empty() || !sciE_.empty()) tree_->Fill();
}

} // namespace ericasim
