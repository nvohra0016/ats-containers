/*
  Copyright 2010-202x held jointly by participating institutions.
  ATS is released under the three-clause BSD License.
  The terms of use and "as is" disclaimer for this license are
  provided in the top-level COPYRIGHT file.

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

//! Downregulates bare soil evaporation through a dessicated zone.
#include "evaporation_downregulation_evaluator.hh"
#include "evaporation_downregulation_model.hh"

namespace Amanzi {
namespace SurfaceBalance {
namespace Relations {

// Constructor from ParameterList
EvaporationDownregulationEvaluator::EvaporationDownregulationEvaluator(
  Teuchos::ParameterList& plist)
  : EvaluatorSecondaryMonotypeCV(plist), consistent_(false)
{
  InitializeFromPlist_();
}

// Virtual copy constructor
Teuchos::RCP<Evaluator>
EvaporationDownregulationEvaluator::Clone() const
{
  return Teuchos::rcp(new EvaporationDownregulationEvaluator(*this));
}


// Initialize by setting up dependencies
void
EvaporationDownregulationEvaluator::InitializeFromPlist_()
{
  // Set up my dependencies
  // - defaults to prefixed via domain
  Tag tag = my_keys_.front().second;
  domain_surf_ = Keys::getDomain(my_keys_.front().first);
  domain_sub_ = Keys::readDomainHint(plist_, domain_surf_, "surface", "domain");

  // dependency: potential_evaporation on surface
  pot_evap_key_ =
    Keys::readKey(plist_, domain_surf_, "potential evaporation", "potential_evaporation");
  dependencies_.insert(KeyTag{ pot_evap_key_, tag });
  rsoil_key_ = Keys::readKey(plist_, domain_surf_, "soil resistance", "soil_resistance");
  dependencies_.insert(KeyTag{ rsoil_key_, tag });
}


void
EvaporationDownregulationEvaluator::Evaluate_(const State& S,
                                              const std::vector<CompositeVector*>& result)
{
  Tag tag = my_keys_.front().second;
  const Epetra_MultiVector& rsoil = 
    *S.Get<CompositeVector>(rsoil_key_, tag).ViewComponent("cell", false);
  const Epetra_MultiVector& pot_evap =
    *S.Get<CompositeVector>(pot_evap_key_, tag).ViewComponent("cell", false);
  Epetra_MultiVector& surf_evap = *result[0]->ViewComponent("cell", false);
  auto& sub_mesh = *S.GetMesh(domain_sub_);
  auto& surf_mesh = *S.GetMesh(domain_surf_);

  for (const auto& region_model : models_) {
    AmanziMesh::Entity_ID_List lc_ids;
    surf_mesh.get_set_entities(
      region_model.first, AmanziMesh::Entity_kind::CELL, AmanziMesh::Parallel_type::OWNED, &lc_ids);

    for (AmanziMesh::Entity_ID sc : lc_ids) {
      auto c = sub_mesh.cells_of_column(sc)[0];
      surf_evap[0][sc] =
        region_model.second->Evaporation(pot_evap[0][sc], rsoil[0][sc]);
    }
  }
}


void
EvaporationDownregulationEvaluator::EvaluatePartialDerivative_(
  const State& S,
  const Key& wrt_key,
  const Tag& wrt_tag,
  const std::vector<CompositeVector*>& result)
{
  Tag tag = my_keys_.front().second;
  if (wrt_key == pot_evap_key_) {
  const Epetra_MultiVector& rsoil = 
    *S.Get<CompositeVector>(rsoil_key_, tag).ViewComponent("cell", false);
    const Epetra_MultiVector& pot_evap =
      *S.Get<CompositeVector>(pot_evap_key_, tag).ViewComponent("cell", false);
    Epetra_MultiVector& surf_evap = *result[0]->ViewComponent("cell", false);
    auto& sub_mesh = *S.GetMesh(domain_sub_);
    auto& surf_mesh = *S.GetMesh(domain_surf_);

    for (const auto& region_model : models_) {
      AmanziMesh::Entity_ID_List lc_ids;
      surf_mesh.get_set_entities(region_model.first,
                                 AmanziMesh::Entity_kind::CELL,
                                 AmanziMesh::Parallel_type::OWNED,
                                 &lc_ids);
      for (AmanziMesh::Entity_ID sc : lc_ids) {
        auto c = sub_mesh.cells_of_column(sc)[0];
        surf_evap[0][sc] = region_model.second->DEvaporationDPotentialEvaporation(
          pot_evap[0][sc], rsoil[0][sc]);
      }
    }
  }
}


void
EvaporationDownregulationEvaluator::EnsureCompatibility_ToDeps_(State& S)
{
  if (!consistent_) {

    Tag tag = my_keys_.front().second;
    S.Require<CompositeVector, CompositeVectorSpace>(pot_evap_key_, tag)
      .SetMesh(S.GetMesh(domain_surf_))
      ->AddComponent("cell", AmanziMesh::Entity_kind::CELL, 1);
    S.Require<CompositeVector, CompositeVectorSpace>(rsoil_key_, tag)
      .SetMesh(S.GetMesh(domain_surf_))
      ->AddComponent("cell", AmanziMesh::Entity_kind::CELL, 1);

    consistent_ = true;
  }
}


} // namespace Relations
} // namespace SurfaceBalance
} // namespace Amanzi
