/*
  Copyright 2010-202x held jointly by participating institutions.
  ATS is released under the three-clause BSD License.
  The terms of use and "as is" disclaimer for this license are
  provided in the top-level COPYRIGHT file.

  Authors: Bo Gao (gaob@ornl.gov)
*/

#pragma once
#include <cmath>

namespace Amanzi {
namespace Flow {
namespace BrooksCoreyFrzCoef {


inline double
frzcoef(double sl, double sg, double sr, double omega)
{
  return 1. - std::exp(-omega * (sl + sg - sr) / (1. - sr)) + std::exp(-omega);
}


inline double
d_frzcoef_dsl(double sl, double sg, double sr, double omega)
{
  return omega / (1. - sr) * std::exp(-omega * (sl + sg - sr) / (1. - sr));
}


inline double
d_frzcoef_dsg(double sl, double sg, double sr, double omega)
{
  return omega / (1. - sr) * std::exp(-omega * (sl + sg - sr) / (1. - sr));
}


} // namespace BrooksCoreyFrzCoef
} // namespace Flow
} // namespace Amanzi
