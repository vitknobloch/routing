//
// Created by knoblvit on 24.2.25.
//
#include "CVRP/cvrp_mutation_random.h"
#include "CVRP/cvrp_individual.h"

CvrpMutationRandom::CvrpMutationRandom(
    const std::shared_ptr<RoutingInstance> &instance) {
  instance_ = instance;
  gen = std::mt19937(rand());

}
bool CvrpMutationRandom::mutate(const std::shared_ptr<Individual> &individual) {
  auto individual_ = std::static_pointer_cast<CvrpIndividual>(individual);

  std::uniform_int_distribution<uint> dist_route(0, individual_->data().size() - 1);
  uint route_start, route_length;
  getRouteStartAndLength(individual_->data(), dist_route(gen), route_start, route_length);
  mutate2optInsideRoute(individual_, route_start, route_length);

  return true;
}

bool CvrpMutationRandom::isInPlace() { return false; }
void CvrpMutationRandom::getRouteStartAndLength(const std::vector<uint> &data, uint search_from,
                                                uint &start, uint &length) {
  uint i = search_from;
  while(data[i] != 0 && data[i] < (uint)instance_->getNodesCount()){
    i = (i + 1) % data.size();
  }
  start = i;
  i = (i+1) % data.size();
  while(data[i] != 0 && data[i] < (uint)instance_->getNodesCount()){
    i = (i + 1) % data.size();
  }
  length = i - start;
}

void CvrpMutationRandom::mutate2optInsideRoute(std::shared_ptr<CvrpIndividual> &individual,
                                               uint start, uint length) {
  if(length < 2)
    return;

  std::uniform_int_distribution<uint> dist(0, length-1);
  uint start_move = wrapWithinSpan(start, length, start, dist(gen));
  uint length_move = dist(gen) + 1;

  if(length_move < 2)
    return;

  auto &data = individual->data();

  const uint end_move = wrapWithinSpan(start, length, start_move, length_move);
  const uint start_node = data[start_move];
  const uint end_node = data[end_move];
  const uint prev_node = data[wrapWithinSpan(start, length, start_move, -1)];
  const uint next_node = data[wrapWithinSpan(start, length, end_move, 1)];

  const uint prev_cost = instance_->getDistance(prev_node, start_node) + instance_->getDistance(end_node, next_node);
  const uint new_cost = instance_->getDistance(prev_node, end_node) + instance_->getDistance(start_node, next_node);

  if(new_cost >= prev_cost){
    return;
  }

  for(uint i = 0; i <= length_move / 2; i++){
    uint left_swap = wrapWithinSpan(start, length, start_move, i);
    uint right_swap = wrapWithinSpan(start, length, end_move, -i);
    uint temp = data[left_swap];
    data[left_swap] = data[right_swap];
    data[right_swap] = temp;
  }

  std::vector<uint> route;
  route.reserve(length);
  uint zero_shift = 0;
  for(uint i = start; i < start + length; i++){
    if(data[i] == 0 || data[i] >= (uint)instance_->getNodesCount()){
      zero_shift = i - start;
      break;
    }
  }

  for(uint i = 0; i < length; i++){
    route.push_back(data[wrapWithinSpan(start, length, start, zero_shift)]);
  }

  for(uint i = 0; i < length; i++){
    data[start + i] = route[i];
  }

  double new_fitness = individual->getFitness() - (double)prev_cost + (double)new_cost;
  individual->setFitness(new_fitness);
}

inline int CvrpMutationRandom::wrapWithinSpan(int begin, int length, int initial,
                                       int shift) {
  return (begin + (initial - begin + shift + length) % length) % (instance_->getNodesCount() + instance_->getVehicleCount());
}
