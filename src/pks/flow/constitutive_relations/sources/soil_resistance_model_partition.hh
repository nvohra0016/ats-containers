/*
  Copyright 2010-202x held jointly by participating institutions.
  ATS is released under the three-clause BSD License.
  The terms of use and "as is" disclaimer for this license are
  provided in the top-level COPYRIGHT file.

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

//! A collection of WRMs along with a Mesh Partition.
/*!

A WRM partition is a list of (region, WRM) pairs, where the regions partition
the mesh.

.. _wrm-partition-typedinline-spec
.. admonition:: wrm-partition-typedinline-spec

   * `"region`" ``[string]`` Region on which the WRM is valid.
   * `"WRM type`" ``[string]`` Name of the WRM type.
   * `"_WRM_type_ parameters`" ``[_WRM_type_-spec]`` See below for the required
     parameter spec for each type.

*/

#ifndef AMANZI_FLOW_RELATIONS_SOIL_RESISTANCE_PARTITION_
#define AMANZI_FLOW_RELATIONS_SOIL_RESISTANCE_PARTITION_

#include "soil_resistance_sakagucki_zeng_model.hh"
#include "MeshPartition.hh"

namespace Amanzi {
namespace Flow {

typedef std::vector<Teuchos::RCP<SoilResistanceSakaguckiZengModel>> 
  SoilResistanceSakaguckiZengModelList;
typedef std::pair<Teuchos::RCP<Functions::MeshPartition>, SoilResistanceSakaguckiZengModelList> 
  SoilResistanceModelPartition;

// Non-member factory
Teuchos::RCP<SoilResistanceModelPartition>
createSoilResistanceModelPartition(Teuchos::ParameterList& plist);

} // namespace Flow
} // namespace Amanzi

#endif
