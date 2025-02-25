//
// Created by knoblvit on 24.2.25.
//
#include "CVRP/cvrp_mutation_random.h"
#include "CVRP/cvrp_individual.h"
#include <cassert>
#include <iostream>

CvrpMutationRandom::CvrpMutationRandom(
    const std::shared_ptr<RoutingInstance> &instance) {
  instance_ = instance;
  gen = std::mt19937(rand());

}
bool CvrpMutationRandom::mutate(const std::shared_ptr<Individual> &individual) {
  auto individual_ = std::static_pointer_cast<CvrpIndividual>(individual);

  std::uniform_int_distribution<uint> dist_route(0, individual_->data().size() - 1);
  uint route_start, route_length;
  uint random_idx = dist_route(gen);
  getRouteStartAndLength(individual_->data(), random_idx, &route_start, &route_length);
  mutate2optInsideRoute(individual_, route_start, route_length);

  return true;
}

bool CvrpMutationRandom::isInPlace() { return false; }
void CvrpMutationRandom::getRouteStartAndLength(const std::vector<uint> &data, uint search_from,
                                                uint *start, uint *length) {
  uint i = search_from;
  while(data[i] > 0 && data[i] < (uint)instance_->getNodesCount()){
    i = (i + 1) % data.size();
  }
  *start = i;
  i = (i+1) % data.size();
  while(data[i] > 0 && data[i] < (uint)instance_->getNodesCount()){
    i = (i + 1) % data.size();
  }
  *length = (i + data.size() - *start) % data.size();
}

void CvrpMutationRandom::mutate2optInsideRoute(std::shared_ptr<CvrpIndividual> &individual,
                                               uint start_route, uint length_route) {
  if(length_route < 3)
    return;

  std::uniform_int_distribution<uint> dist(0, length_route -1);
  uint start_move = wrapWithinSpan(start_route, length_route, start_route, dist(gen));

  std::uniform_int_distribution<uint> dist_len_move(1, length_route -2);
  uint length_move = dist_len_move(gen);

  auto &data = individual->data();

  const uint end_move = wrapWithinSpan(start_route, length_route, start_move, length_move);
  const uint start_node = data[start_move];
  const uint end_node = data[end_move];
  const uint prev_node = data[wrapWithinSpan(start_route, length_route, start_move, -1)];
  const uint next_node = data[wrapWithinSpan(start_route, length_route, end_move, 1)];

  const uint prev_cost = instance_->getDistance(prev_node, start_node) + instance_->getDistance(end_node, next_node);
  const uint new_cost = instance_->getDistance(prev_node, end_node) + instance_->getDistance(start_node, next_node);

  if(new_cost >= prev_cost){
    return;
  }

  for(int i = 0; i < ((int)length_move + 1) / 2; i++){
    uint left_swap = wrapWithinSpan((int)start_route, (int)length_route, (int)start_move, i);
    uint right_swap = wrapWithinSpan((int)start_route, (int)length_route, (int)end_move, -i);
    uint temp = data[left_swap];
    data[left_swap] = data[right_swap];
    data[right_swap] = temp;
  }

  std::vector<uint> route;
  route.reserve(length_route);
  uint zero_shift = 0;
  for(uint i = 0; i < length_route; i++){
    uint node = data[wrapWithinSpan(start_route, length_route, start_route, i)];
    if(node >= (uint)instance_->getNodesCount())
      node = 0;
    if(node == 0){
      zero_shift = i;
      break;
    }
  }

  for(uint i = 0; i < length_route; i++){
    route.push_back(data[wrapWithinSpan(start_route, length_route, start_route, zero_shift + i)]);
  }

  for(uint i = 0; i < length_route; i++){
    data[(start_route + i) % data.size()] = route[i];
  }

  double new_fitness = individual->getFitness() - (double)prev_cost + (double)new_cost;
  individual->setFitness(new_fitness);
}

int CvrpMutationRandom::wrapWithinSpan(int begin, int length, int initial,
                                       int shift) {
  if(initial < begin)
    initial += instance_->getNodesCount() + instance_->getVehicleCount();
  return (begin + (initial - begin + shift + length) % length) % (instance_->getNodesCount() + instance_->getVehicleCount());
}
