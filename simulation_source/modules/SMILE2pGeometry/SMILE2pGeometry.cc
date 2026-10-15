#include "SMILE2pGeometry.hh"

#include <stdexcept>
#include "SMILE_Material.h"
#include "HAPositionManager.h"
#include "SMILE_2p_Geo.h"

namespace ericasim {

SMILE2pGeometry::SMILE2pGeometry()
  : geometryDir_("SMILE_2p_Geo"), geometryType_("ground"),
    haPositionFile_("HAs.pos"), gasPressure_(2.0)
{
}

SMILE2pGeometry::~SMILE2pGeometry() = default;

anlnext::ANLStatus SMILE2pGeometry::mod_define()
{
  VANLGeometry::mod_define();
  define_parameter("geometry_dir", &mod_class::geometryDir_);
  set_parameter_description("directory with the .mqo files (haboc: GeometryCADdir)");
  define_parameter("geometry_type", &mod_class::geometryType_);
  set_parameter_description("ground, flight or no_vessel (haboc: GeometryType)");
  define_parameter("ha_position_file", &mod_class::haPositionFile_);
  set_parameter_description("HA module positions (haboc: HA_PositionFile)");
  define_parameter("gas_pressure", &mod_class::gasPressure_);
  set_parameter_description("TPC gas pressure [atm] (haboc: TPC_Gas_Pressure)");
  return anlnext::AS_OK;
}

G4VUserDetectorConstruction* SMILE2pGeometry::create()
{
  // SMILE_2p::Init(): GeometryType -> geo_flag_ground_0_flight_1
  int flag;
  if      (geometryType_ == "ground")    flag = 0;
  else if (geometryType_ == "flight")    flag = 1;
  else if (geometryType_ == "no_vessel") flag = 2;
  else throw std::invalid_argument("SMILE2pGeometry: unknown geometry_type " + geometryType_);

  material_ = std::make_unique<SMILE_Material>(gasPressure_);
  haPosition_ = std::make_unique<HAPositionManager>();
  haPosition_->LoadPosData(haPositionFile_);
  return new SMILE_2p_Geo(material_.get(), geometryDir_, flag, haPosition_.get());
}

} // namespace ericasim
