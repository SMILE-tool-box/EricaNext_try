#include "ParmaCRPrimaryGen.hh"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

#include <TFile.h>
#include <TH2.h>
#include <TMath.h>

#include "Randomize.hh"
#include "G4SystemOfUnits.hh"

using namespace anlnext;

namespace ericasim {

ParmaCRPrimaryGen::ParmaCRPrimaryGen()
  : planeSize_(3.0 * m), planeDistance_(1.5 * m)   // reference defaults (SMILE_2p.cxx)
{
  setParticleName("neutron");
}

ANLStatus ParmaCRPrimaryGen::mod_define()
{
  BasicPrimaryGen::mod_define();
  hide_parameter("spectral_distribution");
  hide_parameter("energy_min");
  hide_parameter("energy_max");
  hide_parameter("photon_index");
  hide_parameter("energy_mean");
  hide_parameter("energy_sigma");
  hide_parameter("radiation_temperature");

  define_parameter("flux_file", &mod_class::fluxFile_);
  set_parameter_description("PARMA flux ROOT file (macro CR_FluxFile)");
  define_parameter("flux_histogram", &mod_class::fluxHistogram_);
  set_parameter_description("TH2D name; empty: h2d<id> from the particle (neutron 0, proton 1, e- 31, e+ 32, gamma 33)");
  define_parameter("plane_size", &mod_class::planeSize_, LengthUnit(), LengthUnitName());
  set_parameter_description("side of the square generation plane (macro ParaSrcSize)");
  define_parameter("plane_distance", &mod_class::planeDistance_, LengthUnit(), LengthUnitName());
  set_parameter_description("distance of the generation plane from the origin (macro ParaSrcDistance)");
  return AS_OK;
}

ANLStatus ParmaCRPrimaryGen::mod_initialize()
{
  BasicPrimaryGen::mod_initialize();

  // BasicPrimaryGen keeps the particle name private: read the "particle" parameter
  particle_ = get_parameter("particle")->get_value(std::string());
  std::string name = fluxHistogram_;
  if (name.empty()) {
    const std::string& p = particle_;
    if (p == "neutron") name = "h2d0";
    else if (p == "proton") name = "h2d1";
    else if (p == "e-") name = "h2d31";
    else if (p == "e+") name = "h2d32";
    else if (p == "gamma") name = "h2d33";
    else {
      std::cerr << "ParmaCRPrimaryGen: no PARMA histogram for particle " << p
                << " (set flux_histogram)" << std::endl;
      return AS_QUIT_ERROR;
    }
  }

  std::unique_ptr<TFile> file(TFile::Open(fluxFile_.c_str()));
  if (!file || file->IsZombie()) {
    std::cerr << "ParmaCRPrimaryGen: cannot open " << fluxFile_ << std::endl;
    return AS_QUIT_ERROR;
  }
  auto h = dynamic_cast<TH2*>(file->Get(name.c_str()));
  if (h == nullptr) {
    std::cerr << "ParmaCRPrimaryGen: no TH2 " << name << " in " << fluxFile_ << std::endl;
    return AS_QUIT_ERROR;
  }

  // reference GetData(): content x dE x dcos x 2pi ; flattened in the TH2::GetRandom2 order
  const TAxis* ax = h->GetXaxis();
  const TAxis* ay = h->GetYaxis();
  nx_ = h->GetNbinsX();
  const int ny = h->GetNbinsY();
  xLow_.resize(nx_); xWidth_.resize(nx_);
  yLow_.resize(ny); yWidth_.resize(ny);
  for (int i = 0; i < nx_; i++) { xLow_[i] = ax->GetBinLowEdge(i + 1); xWidth_[i] = ax->GetBinWidth(i + 1); }
  for (int j = 0; j < ny; j++) { yLow_[j] = ay->GetBinLowEdge(j + 1); yWidth_[j] = ay->GetBinWidth(j + 1); }
  cumulative_.assign(static_cast<size_t>(nx_) * ny, 0.0);
  double sum = 0.0;
  for (int j = 0; j < ny; j++) {
    for (int i = 0; i < nx_; i++) {
      sum += h->GetBinContent(i + 1, j + 1) * xWidth_[i] * yWidth_[j] * 2 * TMath::Pi();
      cumulative_[i + static_cast<size_t>(nx_) * j] = sum;
    }
  }
  if (!(sum > 0.0)) {
    std::cerr << "ParmaCRPrimaryGen: " << name << " has no positive content" << std::endl;
    return AS_QUIT_ERROR;
  }
  for (auto& c : cumulative_) c /= sum;
  totalFlux_ = sum;

  std::cout << "--------\n"
            << "PrimaryGen status (PARMA cosmic ray)\n"
            << "  particle: " << particle_ << "\n"
            << "  flux: " << fluxFile_ << " : " << name << " (" << h->GetTitle() << ")\n"
            << "  energy range: " << xLow_.front() << " - " << xLow_.back() + xWidth_.back() << " MeV\n"
            << "  total flux: " << totalFlux_ << " /cm2/s\n"
            << "  plane: " << planeSize_ / cm << " cm square at " << planeDistance_ / cm << " cm"
            << std::endl;
  return AS_OK;
}

void ParmaCRPrimaryGen::makePrimarySetting()
{
  // (E, cos theta): TH2::GetRandom2 algorithm with the Geant4 random engine
  const double r1 = G4UniformRand();
  const size_t ibin = std::lower_bound(cumulative_.begin(), cumulative_.end(), r1) - cumulative_.begin();
  const int iy = static_cast<int>(ibin / nx_);
  const int ix = static_cast<int>(ibin - static_cast<size_t>(nx_) * iy);
  const double ene = (xLow_[ix] + xWidth_[ix] * G4UniformRand()) * MeV;
  const double costheta = yLow_[iy] + yWidth_[iy] * G4UniformRand();

  // reference GeneratePrimaries()
  const double theta = std::acos(costheta);
  const double phi = 2.0 * 3.14159265358 * G4UniformRand();
  G4ThreeVector dir(1., 0., 0.);
  dir.setMag(1.0);
  dir.setTheta(theta);
  dir.setPhi(phi);

  const double px = planeSize_ * (G4UniformRand() - 0.5);
  const double py = planeSize_ * (G4UniformRand() - 0.5);
  G4ThreeVector pos(px, py, -planeDistance_);
  pos.rotateY(dir.getTheta());
  pos.rotateZ(dir.getPhi());

  setPrimary(pos, ene, dir);
}

ANLStatus ParmaCRPrimaryGen::mod_end_run()
{
  const double area = planeSize_ * planeSize_;
  const double realTime = Number() / ((area / cm2) * totalFlux_) * s;
  setRealTime(realTime);
  std::cout << "ParmaCRPrimaryGen::mod_end_run\n"
            << "  Number: " << Number() << "\n"
            << "  Total flux: " << totalFlux_ << " /cm2/s\n"
            << "  Area: " << area / cm2 << " cm2\n"
            << "  Real time: " << realTime / s << " s" << std::endl;
  return AS_OK;
}

} // namespace ericasim
