/*
  Copyright 2010-202x held jointly by participating institutions.
  ATS is released under the three-clause BSD License.
  The terms of use and "as is" disclaimer for this license are
  provided in the top-level COPYRIGHT file.

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

//! A simple model for allowing porosity to vary with pressure.
/*!

Based on a linear increase, i.e.

.. math::

   \phi = \phi_{base} + H(p - p_{atm}) * \alpha

where :math:`H` is the heaviside function and :math:`\alpha` is the provided
compressibility.  If the inflection point is set to zero, the above function is
exact.  However, then the porosity function is not smooth (has discontinuous
derivatives), so the inflection point smooths this with a quadratic that
matches the value and derivative at the inflection point and is 0 with 0 slope
at atmospheric pressure.

.. _compressible-porosity-standard-model-spec
.. admonition:: compressible-porosity-standard-model-spec

   * `"region`" ``[string]`` Region on which this is applied.
   * `"pore compressibility [Pa^-1]`" ``[double]``  :math:`\alpha` as described above
   * `"pore compressibility inflection point [Pa]`" ``[double]`` **1000**

  The inflection point above which the function is linear.

Example:

.. code-block:: xml

  <ParameterList name="soil" type="ParameterList">
    <Parameter name="region" type="string" value="soil" />
    <Parameter name="pore compressibility [Pa^-1]" type="double" value="1.e-9" />
    <Parameter name="pore compressibility inflection point [Pa]" type="double" value="1000." />
  </ParameterList>

*/

#ifndef AMANZI_FLOWRELATIONS_SOIL_RESISTANCE_SAKAGUCKI_ZENG_MODEL_HH_
#define AMANZI_FLOWRELATIONS_SOIL_RESISTANCE_SAKAGUCKI_ZENG_MODEL_HH_

#include "Teuchos_ParameterList.hpp"
#include "dbc.hh"

namespace Amanzi {
namespace Flow {

class SoilResistanceSakaguckiZengModel {
 public:
  explicit SoilResistanceSakaguckiZengModel(Teuchos::ParameterList& plist) : plist_(plist)
  {
    InitializeFromPlist_();
  }


  double RsoilbySakagickiZeng(double saturation_gas, double porosity)
  {
    double r_soil; 
    if (saturation_gas == 0.) {
        r_soil = 0.; // ponded water
    } else {
    double vp_diffusion = 2.2e-5 * std::pow(porosity, 2) * std::pow(1 - sr_, 2 + 3 * b_);
    double L_Rsoil = d_ * (std::exp(std::pow(saturation_gas, 5)) - 1) / (std::exp(1) - 1);
    r_soil = L_Rsoil / vp_diffusion;
  }
  AMANZI_ASSERT(r_soil >= 0);
  return r_soil;
  }


  double DRsoilbySakagickiZengDSatGas(double saturation_gas, double porosity)
  {
    double vp_diffusion = 2.2e-5 * std::pow(porosity, 2) * std::pow(1 - sr_, 2 + 3 * b_);
    double coef = d_ / (std::exp(1) - 1) / vp_diffusion;
    return coef * std::exp(std::pow(saturation_gas, 5)) * 5 * std::pow(saturation_gas, 4);
  }


  double DRsoilbySakagickiZengDPorosity(double saturation_gas, double porosity)
  {
    double L_Rsoil = d_ * (std::exp(std::pow(saturation_gas, 5)) - 1) / (std::exp(1) - 1);
    double coef = L_Rsoil / 2.2e-5 * std::pow(1 - sr_, -2 - 3 * b_);
    return -2 * coef * std::pow(porosity, -3);
  }

 protected:
  void InitializeFromPlist_()
  {
    d_ = plist_.get<double>("dessicated zone thickness [m]", 0.1);
    sr_ = plist_.get<double>("residual saturation [-]");

    if (plist_.get<std::string>("WRM Type") == "van Genuchten") {
      if (plist_.isParameter("van Genuchten m [-]")) {
        double m = plist_.get<double>("van Genuchten m [-]");
        double n = 1.0 / (1.0 - m);
        double lambda = (n - 1) * (1 - std::pow(0.5, n / (n - 1)));
        b_ = 1. / lambda;
      } else {
        double n = plist_.get<double>("van Genuchten n [-]");
        double lambda = (n - 1) * (1 - std::pow(0.5, n / (n - 1)));
        b_ = 1. / lambda;
      } 
    } else if (plist_.get<std::string>("WRM Type") == "Brooks-Corey") {
        double lambda = plist_.get<double>("Brooks Corey lambda [-]");
        b_ = 1. / lambda;
    } else if (plist_.get<std::string>("WRM Type") == "Clapp-Hornberger") {
        b_ = plist_.get<double>("Clapp-Hornberger b [-]");
    } 
  }

 protected:
  Teuchos::ParameterList plist_;
  double sr_;
  double d_;
  double b_;
};

} // namespace Flow
} // namespace Amanzi

#endif
