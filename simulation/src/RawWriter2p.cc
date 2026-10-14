#include "RawWriter2p.hh"
#include "EdepPickUp.hh"
#include "TPCEmulator.hh"
#include "HAEmulator.hh"

#include <filesystem>
#include <iostream>
#include <vector>

#include <TFile.h>
#include <TTree.h>

#include "TPC_FileManager.h"
#include "HA_FileManager.h"
#include "HA_common.h"

using namespace anlnext;

namespace ericasim {

RawWriter2p::RawWriter2p()
  : outputDir_("rawdata"), writeHA_(true), writeAnode_(true), writeCathode_(true) {}

RawWriter2p::~RawWriter2p() = default;

ANLStatus RawWriter2p::mod_define()
{
  define_parameter("output_dir", &mod_class::outputDir_);
  set_parameter_description("directory of the raw files (created; EmuWrite2p: rawdata)");
  define_parameter("Write_HA_Data", &mod_class::writeHA_);
  define_parameter("Write_Anode_Data", &mod_class::writeAnode_);
  define_parameter("Write_Cathode_Data", &mod_class::writeCathode_);
  define_parameter("truth_file", &mod_class::truthFile_);
  set_parameter_description("ROOT file of the sim tree (event ID, primary, deposits); empty: not written");
  return AS_OK;
}

ANLStatus RawWriter2p::mod_initialize()
{
  get_module("EdepPickUp", &edep_);
  if (writeAnode_ || writeCathode_) get_module("TPCEmulator", &tpc_);
  if (writeHA_) get_module("HAEmulator", &ha_);

  std::filesystem::create_directories(outputDir_);
  if (writeHA_)
    for (int haid = 0; haid < HAEmulator::N_HA; haid++)
      haFM_[haid] = std::make_unique<HA_FileManager>(outputDir_.c_str(),
                                                     HA_common::Get_HA_ID(haid, HA_common::ID_TO_NAME));
  for (int ac = 0; ac < 2; ac++) {
    if ((ac == 0 && !writeAnode_) || (ac == 1 && !writeCathode_)) continue;
    for (int bid = 0; bid < tpc_->NBoard(); bid++)
      tpcFM_[ac][bid] = std::make_unique<TPC_FileManager>(outputDir_.c_str(), bid + 1, ac);
  }

  if (!truthFile_.empty()) {
    truth_ = new TFile(truthFile_.c_str(), "recreate");
    tree_ = new TTree("sim", "simulation tree (events written to the raw files)");
    tree_->Branch("EventID", &evid_, "EventID/I");
    tree_->Branch("SrcEng", &srcEng_, "SrcEng/D");
    tree_->Branch("SrcPos", srcPos_, "SrcPos[3]/D");
    tree_->Branch("SrcDir", srcDir_, "SrcDir[3]/D");
    tree_->Branch("E_psa", &ePSA_, "E_psa/D");
    tree_->Branch("E_tpc", &eTPC_, "E_tpc/D");
    tree_->Branch("E_anti", &eAnti_, "E_anti/D");
  }
  return AS_OK;
}

ANLStatus RawWriter2p::mod_analyze()
{
  const unsigned evid = (unsigned)edep_->EventID();
  if (writeHA_) {
    for (int haid = 0; haid < HAEmulator::N_HA; haid++)
      haFM_[haid]->write_event(evid, const_cast<unsigned short*>(ha_->ADC(haid)));
  }
  for (int ac = 0; ac < 2; ac++) {
    if ((ac == 0 && !writeAnode_) || (ac == 1 && !writeCathode_)) continue;
    for (int bid = 0; bid < tpc_->NBoard(); bid++) {
      std::vector<unsigned> hit(tpc_->Hits(ac, bid));
      tpcFM_[ac][bid]->write_event(evid, tpc_->EventTime(), hit.data(),
                                   const_cast<unsigned short*>(tpc_->FADC(ac, bid)), tpc_->NHit(ac, bid));
    }
  }
  if (tree_) {
    evid_ = (int)evid;
    srcEng_ = edep_->PrimaryEnergy();
    for (int i = 0; i < 3; i++) { srcPos_[i] = edep_->PrimaryPos()[i]; srcDir_[i] = edep_->PrimaryDir()[i]; }
    ePSA_ = edep_->ScintiEnergy();
    eTPC_ = edep_->TPCEnergy();
    eAnti_ = edep_->AntiEnergy();
    tree_->Fill();
  }
  nWritten_++;
  return AS_OK;
}

ANLStatus RawWriter2p::mod_finalize()
{
  for (auto& side : tpcFM_)
    for (auto& fm : side) fm.reset();   // closes the files
  for (auto& fm : haFM_) fm.reset();
  if (truth_) {
    truth_->cd();
    tree_->Write();
    truth_->Close();
    delete truth_;
    truth_ = nullptr;
  }
  std::cout << "RawWriter2p: " << nWritten_ << " events written to " << outputDir_ << std::endl;
  return AS_OK;
}

} // namespace ericasim
