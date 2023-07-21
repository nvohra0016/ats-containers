/*
  Copyright 2010-202x held jointly by participating institutions.
  ATS is released under the three-clause BSD License.
  The terms of use and "as is" disclaimer for this license are
  provided in the top-level COPYRIGHT file.

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

//! Downregulates bare soil evaporation through a dessicated zone.
/*!

Calculates evaporative resistance through a dessicated zone.

Sakagucki and Zeng 2009 equations 9 and 10.

Requires two parameters,


* `"dessicated zone thickness [m]`" ``[double]`` Thickness over which vapor must diffuse
  when the soil is dry.

* `"Clapp and Hornberger b [-]`" ``[double]`` Exponent of the Clapp & Hornberger curve for
  the top layer of soil.  Nominally this could probably be pulled from van
  Genuchten curves that we typically use, but it doesn't appear to be the most
  important parameter.

* `"Soil resistance method`" ``[string]`` Soil resistance model, currently support 
  {sakagucki_zeng, sellers}, default is always sakagucki_zeng.

These may be provided via parameter list or LandCover type.

*/

#include "Teuchos_ParameterList.hpp"

namespace Amanzi {
namespace SurfaceBalance {
namespace Relations {

class EvaporationDownregulationModel {
 public:
  explicit EvaporationDownregulationModel(Teuchos::ParameterList& plist) {}

  double Evaporation(double pot_evap, double rsoil)
  {
    return pot_evap / (1. + rsoil);
  }

  double DEvaporationDPotentialEvaporation(double pot_evap, double rsoil)
  {
    return 1. / (1. + rsoil);
  }

  double DEvaporationDRsoil(double pot_evap, double rsoil)
  {
    return -pot_evap * std::pow(1. + rsoil, -2);
  }

};

} // namespace Relations
} // namespace SurfaceBalance
} // namespace Amanzi
