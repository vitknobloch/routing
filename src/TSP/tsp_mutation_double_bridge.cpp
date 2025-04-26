#include "TSP/tsp_mutation_double_bridge.h"
#include "TSP/tsp_individual.h"
#include <cassert>

TspMutationDoubleBridge::TspMutationDoubleBridge() : rand(), gen(rand()){
  mutation_rate_ = 1.0;
}
bool TspMutationDoubleBridge::isInPlace() { return false; }
double TspMutationDoubleBridge::getMutationRate() { return mutation_rate_;}
void TspMutationDoubleBridge::setMutationRate(double mutation_rate) {
  assert(mutation_rate >= 0.0 && mutation_rate <= 1.0);
  mutation_rate_ = mutation_rate;
}

bool TspMutationDoubleBridge::mutate(
    const std::shared_ptr<Individual> &individual) {
  auto individual_ = std::static_pointer_cast<TspIndividualStructured>(individual);
  if(individual_->getData().size() < 4)
    return true;

  uint half = individual_->getData().size() / 2;
  std::uniform_int_distribution<uint> dist1(0, half);
  std::uniform_int_distribution<uint> dist2(0, half - 1);
  std::uniform_int_distribution<uint> dist3(half + 1, individual_->getData().size() - 1);
  std::uniform_int_distribution<uint> dist4(half + 1, individual_->getData().size() - 2);

  uint first = dist1(gen);
  uint second = dist2(gen);
  if(second >= first)
    second++;
  uint third = dist3(gen);
  uint fourth = dist4(gen);
  if(fourth >= third)
    fourth++;

  TspIndividualSegment segment1{std::min(first, second), std::max(first, second)};
  TspIndividualSegment segment2{std::min(third, fourth), std::max(third, fourth)};

  individual_->performDoubleBridgeMove(segment1, segment2);

  return true;
}
