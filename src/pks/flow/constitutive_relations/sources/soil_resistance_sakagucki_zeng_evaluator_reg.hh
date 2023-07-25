/*
  Copyright 2010-202x held jointly by participating institutions.
  ATS is released under the three-clause BSD License.
  The terms of use and "as is" disclaimer for this license are
  provided in the top-level COPYRIGHT file.

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

/*
  Evaluates the soil resistance at top cells through the Sakagucki-Zeng model.
*/


#include "soil_resistance_sakagucki_zeng_evaluator.hh"

namespace Amanzi {
namespace Flow {

// registry of method
Utils::RegisteredFactory<Evaluator, SoilResistanceSakaguckiZengEvaluator>
  SoilResistanceSakaguckiZengEvaluator::fac_("sakagucki-zeng soil resistance");

} // namespace Flow
} // namespace Amanzi
