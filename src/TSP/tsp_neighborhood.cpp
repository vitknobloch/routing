//
// Created by knoblvit on 22.4.25.
//
#include "TSP/tsp_neighborhood.h"

TspNeighborhood::TspNeighborhood() = default;

Neighborhood::SearchResult
TspNeighborhood::search(const std::shared_ptr<Individual> &individual) {
  const std::shared_ptr<TspIndividualStructured> &individual_ = std::static_pointer_cast<TspIndividualStructured>(individual);
  const uint size = individual_->getData().size();
  bool performed = false;
  for(uint i = 0; i < size - 1; i++){
    for(uint j = i + 1; j < size; j++){
      TspIndividualSegment segment{i, j};
      if(individual_->test2optMove(segment)){
        individual_->perform2optMove(segment);
        performed = true;
      }
    }
  }
  return performed ? Neighborhood::SearchResult::IMPROVED : Neighborhood::SearchResult::EXHAUSTED;
}

void TspNeighborhood::reset(const std::shared_ptr<Individual> &individual) {}

