#include "TSP/tsp_neighborhood.h"
#include <cassert>

TspNeighborhood::TspNeighborhood() : rand_(), gen_(rand_()), dist_(0.0, 1.0) {
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    exhausted_[i] = false;
    try_count[i] = 1;
    success_count[i] = 1;
  }
}

Neighborhood::SearchResult
TspNeighborhood::search(const std::shared_ptr<Individual> &individual) {
  const auto &individual_ = std::static_pointer_cast<TspIndividualStructured>(individual);
  neighborhood_options selected = selectNeighborhoodOption();
  if(selected == neighborhood_options::SIZE)
    return Neighborhood::SearchResult::EXHAUSTED;

  bool result = false;
  if(selected == neighborhood_options::TWO_OPT)
    result = perform2opt(individual_);
  else if(selected == neighborhood_options::SWAP)
    result = performSwap(individual_);
  else if(selected == neighborhood_options::RELOCATE)
    result = performRelocate(individual_);

  if(result){
    success(selected);
    return Neighborhood::SearchResult::IMPROVED;
  }
  else{
    failure(selected);
    return Neighborhood::SearchResult::UNIMPROVED;
  }
}

void TspNeighborhood::reset(const std::shared_ptr<Individual> &individual) {
  for(int i = 0; i < neighborhood_options::SIZE; i++)
    exhausted_[i] = false;
}
TspNeighborhood::neighborhood_options
TspNeighborhood::selectNeighborhoodOption() {
  double max_val = 0;
  double vals[neighborhood_options::SIZE];
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    if(!exhausted_[i])
      max_val += success_count[i] / try_count[i];
    vals[i] = max_val;
  }

  if(max_val == 0)
    return neighborhood_options::SIZE;
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    vals[i] /= max_val;
  }

  double choice = dist_(gen_);
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    if(choice <= vals[i])
      return static_cast<neighborhood_options>(i);
  }

  assert(false);
  return neighborhood_options::SIZE; // This shouldn't happen
}

void TspNeighborhood::success(
    TspNeighborhood::neighborhood_options neighborhood) {
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    exhausted_[i] = false;
  }
  try_count[neighborhood] += 1;
  success_count[neighborhood] += 1;
}

void TspNeighborhood::failure(
    TspNeighborhood::neighborhood_options neighborhood) {
  exhausted_[neighborhood] = true;
  try_count[neighborhood] += 1;
}

bool TspNeighborhood::perform2opt(
    const std::shared_ptr<TspIndividualStructured> &individual) {
  const uint size = individual->getData().size();
  bool performed = false;
  for(uint i = 0; i < size - 1; i++){
    for(uint j = i + 1; j < size; j++){
      TspIndividualSegment segment{i, j};
      if(individual->test2optMove(segment)){
        individual->perform2optMove(segment);
        performed = true;
      }
    }
  }
  return performed;
}
bool TspNeighborhood::performSwap(
    const std::shared_ptr<TspIndividualStructured> &individual) {
  const uint size = individual->getData().size();
  bool performed = false;
  for(uint i = 0; i < size - 2; i++){
    for(uint j = i + 2; j < size; j++){
      if(individual->testSwapMove(i, j)){
        individual->performSwapMove(i, j);
        performed = true;
      }
    }
  }
  return performed;
}

bool TspNeighborhood::performRelocate(
    const std::shared_ptr<TspIndividualStructured> &individual) {
  const uint size = individual->getData().size();
  bool performed = false;
  for(uint i = 0; i < size; i++){
    for(uint j = 0; j < size; j++){
      if(i == j)
        continue;
      if(individual->testRelocateMove(i, j)){
        individual->performRelocateMove(i, j);
        performed = true;
      }
    }
  }
  return performed;
}
