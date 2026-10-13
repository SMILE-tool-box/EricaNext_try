#ifndef EricaSim_ParmaCRPrimaryGen_H
#define EricaSim_ParmaCRPrimaryGen_H 1

#include <string>
#include <vector>
#include "BasicPrimaryGen.hh"

namespace ericasim {

// Port of SMILE_2p_PA_CR (SMILE_tool_box_v2.20/Erica/SMILE_2p, macro "SourceType : CR"):
// cosmic-ray particles with the energy / zenith-angle distribution of a PARMA flux file.
//  - flux file: TH2D "h2d<id>" (x: E [MeV], y: cos(theta), content: /cm2/s/sr/MeV) with
//    id = 0 neutron, 1 proton, 31 e-, 32 e+, 33 gamma (reference GetData());
//    flux_histogram overrides the name (reference "ling_gamma": particle gamma, "hist_atmos")
//  - (E, cos theta) are drawn from content x dE x dcos x 2pi, as TH2::GetRandom2 does
//    (bin by the cumulative sum, then uniform inside the bin), but with the Geant4 random
//    engine instead of gRandom (the reference reseeded gRandom from the CLHEP seeds every event)
//  - direction: polar angle acos(cos theta), azimuth uniform; the start point is uniform on a
//    square of side plane_size, perpendicular to the direction, at plane_distance upstream of
//    the origin (reference ParaSrcSize / ParaSrcDistance)
// At the end of the run the equivalent exposure time N / (plane area x total flux) is printed
// and set as the real time.
class ParmaCRPrimaryGen : public anlgeant4::BasicPrimaryGen
{
  DEFINE_ANL_MODULE(ParmaCRPrimaryGen, 1.0);
public:
  ParmaCRPrimaryGen();

  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_end_run() override;

  void makePrimarySetting() override;

  double TotalFlux() const { return totalFlux_; }   // [/cm2/s] (sum of content x dE x dcos x 2pi)

private:
  std::string particle_;
  std::string fluxFile_;
  std::string fluxHistogram_;
  double planeSize_;
  double planeDistance_;

  // flattened bins (i = ix + nx * iy) and their cumulative probability
  std::vector<double> cumulative_;
  std::vector<double> xLow_, xWidth_, yLow_, yWidth_;
  int nx_ = 0;
  double totalFlux_ = 0.0;
};

} // namespace ericasim

#endif
