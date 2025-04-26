#include "VRP-TW/vrptw_mutation_reinsert.h"
#include <cassert>
VrptwMutationReinsert::VrptwMutationReinsert() : rand(), gen(rand()){
  mutation_rate_ = 0;
}
bool VrptwMutationReinsert::isInPlace() { return false; }

bool VrptwMutationReinsert::mutate(
    const std::shared_ptr<Individual> &individual) {
  auto individual_ = std::static_pointer_cast<VrptwIndividualStructured>(individual);
  auto source = selectSource(individual_);
  auto target = selectTarget(individual_, source);
  individual_->performRelocateMove(source, target);
  return true;
}

double VrptwMutationReinsert::getMutationRate() { return mutation_rate_; }

void VrptwMutationReinsert::setMutationRate(double mutation_rate) {
  assert(mutation_rate >= 0.0 && mutation_rate <= 1.0);
  mutation_rate_ = mutation_rate;
}

VrptwRouteSegment VrptwMutationReinsert::selectSource(
    const std::shared_ptr<VrptwIndividualStructured> &individual) {
  const auto &routes = individual->getRoutes();
  uint unempty_vehicles_count = 0;
  for(const auto &route : routes){
    if(!route.customers.empty())
      unempty_vehicles_count++;
  }
  std::uniform_int_distribution<uint> dist(0, unempty_vehicles_count - 1);
  uint vehicle_rand = dist(gen);
  uint vehicle_idx = 0;
  for(const auto &route : routes){
    if(!route.customers.empty()){
      if(vehicle_rand == 0){
        break;
      }
      vehicle_rand--;
    }
    vehicle_idx++;
  }

  const auto &route = routes[vehicle_idx];
  std::uniform_int_distribution<uint> dist2(0, route.customers.size() - 1);
  uint customer_rand = dist2(gen);

  const uint max_length = route.customers.size() - customer_rand;
  std::uniform_int_distribution<uint> dist3(1, max_length);
  uint length_rand = dist3(gen);
  return VrptwRouteSegment{vehicle_idx, customer_rand, length_rand};
}
VrptwRouteSegment VrptwMutationReinsert::selectTarget(
    const std::shared_ptr<VrptwIndividualStructured> &individual,
    const VrptwRouteSegment &source) {
  const auto &routes = individual->getRoutes();
  std::uniform_int_distribution<uint> dist1(0, routes.size() - 1);
  uint vehicle_rand = dist1(gen);
  while(vehicle_rand == source.route_idx){
    vehicle_rand = dist1(gen);
  }
  std::uniform_int_distribution<uint> dist2(0, routes[vehicle_rand].customers.size());
  uint customer_rand = dist2(gen);
  return VrptwRouteSegment{vehicle_rand, customer_rand, 0};
}
