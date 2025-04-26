#include "CVRP/cvrp_mutation_random.h"
#include "CVRP/cvrp_individual.h"
#include <cassert>
#include <iostream>

CvrpMutationRandom::CvrpMutationRandom(
    const std::shared_ptr<RoutingInstance> &instance) {
  instance_ = instance;
  gen = std::mt19937(rand());
  mutation_rate_ = 1.0;
}
bool CvrpMutationRandom::mutate(const std::shared_ptr<Individual> &individual) {
  auto individual_ = std::static_pointer_cast<CvrpIndividual>(individual);

  std::uniform_real_distribution<double> dist_mutation(0.0, 1.0);
  double selection = dist_mutation(gen);
  if(individual_->getTotalConstraintViolation() > 0){
    if(selection < 0.45){
      mutateKick(individual_);
    }
    else if( selection < 0.9){
      mutateExchangeDemandViolated(individual_);
    }else{
      mutate2opt(individual_);
    }
  }
  else{
    if(selection < 0.1){
      mutateKick(individual_);
    }
    else if(selection < 0.25){
      mutateExchange(individual_);
    }else if(selection < 0.9){
      std::uniform_int_distribution<uint> dist_route(0, individual_->data().size() - 1);
      uint route_start, route_length;
      uint random_idx = dist_route(gen);
      getRouteStartAndLength(individual_->data(), random_idx, &route_start, &route_length);
      mutate2optInsideRoute(individual_, route_start, route_length);
    }else{
      mutate2opt(individual_);
    }
  }

  return true;
}

bool CvrpMutationRandom::isInPlace() { return true; }
void CvrpMutationRandom::getRouteStartAndLength(const std::vector<uint> &data, uint search_from,
                                                uint *start, uint *length) {
  uint i = search_from;
  while(isCustomer(data[i])){
    i = (i + 1) % data.size();
  }
  *start = i;
  i = (i+1) % data.size();
  while(isCustomer(data[i])){
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
    if(isDepot(node)){
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
    initial += instance_->getNodesCount() + instance_->getVehicleCount() - 1;
  return (begin + (initial - begin + shift + length) % length) % (instance_->getNodesCount() + instance_->getVehicleCount() - 1);
}

void CvrpMutationRandom::mutateKick(
    std::shared_ptr<CvrpIndividual> &individual) {
  auto &data = individual->data();
  uint kicked_idx = selectIdxToKick(individual);
  int insert_idx = selectInsertIndex(individual, kicked_idx);
  if(insert_idx == -1)
    return;
  uint kicked_node = data[kicked_idx];
  assert(isCustomer(kicked_node));

  uint kicked_prev_node = data[(kicked_idx + data.size() - 1) % data.size()];
  uint kicked_next_node = data[(kicked_idx + 1) % data.size()];
  int kick_cost = instance_->getDistance(kicked_prev_node, kicked_next_node) - instance_->getDistance(kicked_prev_node, kicked_node) - instance_->getDistance(kicked_node, kicked_next_node);

  uint insert_prev_node = data[insert_idx];
  uint insert_next_node = data[(insert_idx + 1) % data.size()];
  int insert_cost = instance_->getDistance(insert_prev_node, kicked_node) + instance_->getDistance(kicked_node, insert_next_node) - instance_->getDistance(insert_prev_node, insert_next_node);

  if(kick_cost + insert_cost > 0){
    return;
  }

  if (kicked_idx < (uint)insert_idx) {
    // Shift left
    for (int i = kicked_idx; i < insert_idx; ++i) {
      data[i] = data[i + 1];
    }
  } else {
    // Shift right
    for (int i = kicked_idx; i > insert_idx; --i) {
      data[i] = data[i - 1];
    }
  }
  data[insert_idx] = kicked_node;

  //assert(individual->assertData());
  individual->resetEvaluated();
  individual->evaluate();
}

uint CvrpMutationRandom::selectIdxToKick(
    std::shared_ptr<CvrpIndividual> &individual) {
  auto &data = individual->data();
  std::uniform_int_distribution<uint> dist_kick(0, data.size() - 1);
  uint idx = dist_kick(gen);

  if(individual->getTotalConstraintViolation() > 0){
    return selectIdxToKickDemandViolated(individual, idx);
  }

  while(isDepot(data[idx]))
    idx = dist_kick(gen);

  return idx;
}

uint CvrpMutationRandom::selectIdxToKickDemandViolated(std::shared_ptr<CvrpIndividual> &individual, uint start_idx){
  const auto &nodes = instance_->getNodes();
  auto &data = individual->data();
  uint first_vehicle_start = data.size();
  uint last_vehicle_start = data.size();
  uint i = start_idx;
  uint demand_running_sum = 0;
  while(i != first_vehicle_start){
    const uint node = isDepot(data[i]) ? 0 : data[i];
    if(node == 0){
      if(first_vehicle_start == data.size()){
        first_vehicle_start = i;
        last_vehicle_start = i;
        demand_running_sum = 0;
      }
      uint demand_violation = std::max((int)demand_running_sum - instance_->getVehicleCapacity(), 0);
      if(demand_violation > 0){
        break;
      }
      demand_running_sum = 0;
      last_vehicle_start = i;
    }
    demand_running_sum += nodes[node].demand;
    i = (i + 1) % data.size();
  }

  assert(demand_running_sum > (uint)instance_->getVehicleCapacity());
  const uint length = (i + 2 * data.size() - last_vehicle_start - 1) % data.size();

  std::uniform_int_distribution<uint> dist(1, length);
  return (dist(gen) + last_vehicle_start) % data.size();
}

int CvrpMutationRandom::selectInsertIndex(
    std::shared_ptr<CvrpIndividual> &individual, uint kicked_idx) {
  const auto &nodes = instance_->getNodes();
  auto &data = individual->data();
  const uint kicked_node = data[kicked_idx];
  assert(isCustomer(kicked_node));
  const uint kicked_demand = nodes[kicked_node].demand;

  std::uniform_int_distribution<uint> dist_kick(0, data.size() - 1);
  uint start_idx = dist_kick(gen);

  uint first_vehicle_start = data.size();
  uint last_vehicle_start = 0;
  uint i = start_idx;
  uint demand_running_sum = instance_->getVehicleCapacity() + 1;
  while(i != first_vehicle_start) {
    const uint node = isDepot(data[i]) ? 0 : data[i];
    if(node == kicked_node){
      demand_running_sum += instance_->getVehicleCapacity() + 1;
    }
    if (node == 0) {
      if (first_vehicle_start == data.size()) {
        first_vehicle_start = i;
        last_vehicle_start = i;
      }
      if (kicked_demand + demand_running_sum <=
          (uint)instance_->getVehicleCapacity()) {
        break;
      }
      demand_running_sum = 0;
      last_vehicle_start = i;
    }
    demand_running_sum += nodes[node].demand;
    i = (i + 1) % data.size();
  }

  if(demand_running_sum + kicked_demand > (uint)instance_->getVehicleCapacity())
    return -1;

  uint best_insert_idx = last_vehicle_start;
  int best_insert_cost = std::numeric_limits<int>::max();
  uint vehicle_end = i;
  i = last_vehicle_start;
  while(i != vehicle_end){
    const uint prev_node = data[i];
    const uint next_node = data[(i+1) % data.size()];
    int insert_cost = instance_->getDistance(prev_node, kicked_node) + instance_->getDistance(kicked_node, next_node) - (int)instance_->getDistance(prev_node, next_node);
    if(insert_cost < best_insert_cost){
      best_insert_cost = insert_cost;
      best_insert_idx = i;
    }
    i = (i + 1) % data.size();
  }
  return best_insert_idx;
}

void CvrpMutationRandom::mutateExchange(
    std::shared_ptr<CvrpIndividual> &individual) {
  auto &data = individual->data();
  uint kicked_idx = selectIdxToKick(individual);

  int insert_idx = selectIdxToExchange(individual, kicked_idx);
  if(insert_idx == -1)
    return;
  uint kicked_node = data[kicked_idx];
  uint exchange_node = data[insert_idx];
  assert(isCustomer(kicked_node));
  assert(isCustomer(exchange_node));


  uint prev_kicked = data[(kicked_idx + data.size() - 1) % data.size()];
  uint next_kicked = data[(kicked_idx + 1) % data.size()];
  uint prev_node = data[(insert_idx + data.size() - 1) % data.size()];
  uint next_node = data[(insert_idx + 1) % data.size()];
  int cost = instance_->getDistance(prev_node, kicked_node) + instance_->getDistance(kicked_node, next_node);
  cost += instance_->getDistance(prev_kicked, exchange_node) + instance_->getDistance(exchange_node, next_kicked);
  cost -= instance_->getDistance(prev_node, exchange_node) + instance_->getDistance(exchange_node, next_node);
  cost -= instance_->getDistance(prev_kicked, kicked_node) + instance_->getDistance(kicked_node, next_kicked);


  if(cost > 0){
    return;
  }

  uint temp = data[kicked_idx];
  data[kicked_idx] = data[insert_idx];
  data[insert_idx] = temp;

  //assert(individual->assertData());
  individual->resetEvaluated();
  individual->evaluate();
}
int CvrpMutationRandom::selectIdxToExchange(
    std::shared_ptr<CvrpIndividual> &individual, uint kicked_idx) {
  const auto &nodes = instance_->getNodes();
  auto &data = individual->data();
  const uint kicked_node = data[kicked_idx];
  const uint kicked_demand = nodes[kicked_node].demand;
  const uint prev_kicked = data[(kicked_idx - 1 + data.size()) % data.size()];
  const uint next_kicked = data[(kicked_idx+1) % data.size()];
  assert(isCustomer(kicked_node));
  uint i = kicked_idx;

  while(!isDepot(data[i])){
    i = (i - 1 + data.size()) % data.size();
  }
  uint kicked_vehicle_start = i;
  uint kicked_vehicle_demand = 0;
  i = (i+1) % data.size();
  while(!isDepot(data[i])){
    kicked_vehicle_demand += nodes[data[i]].demand;
    i = (i+1) % data.size();
  }
  kicked_vehicle_demand -= kicked_demand;
  int max_removed_demand = instance_->getVehicleCapacity() - kicked_vehicle_demand;

  uint last_vehicle_start = i;
  int best_cost = 0;
  int best_idx = -1;
  uint min_demand = instance_->getVehicleCapacity();
  uint max_demand = 0;
  uint vehicle_demand = 0;
  while(i != kicked_vehicle_start){
    i = (i+1) % data.size();
    if(!isDepot(data[i])){
      vehicle_demand += nodes[data[i]].demand;
      if(nodes[data[i]].demand > (int)max_demand)
        max_demand = nodes[data[i]].demand;
      if(nodes[data[i]].demand < (int)min_demand)
        min_demand = nodes[data[i]].demand;
      if(kicked_idx == i){
        vehicle_demand += instance_->getVehicleCapacity();
      }
    }
    else{
      int min_removed_demand = vehicle_demand + kicked_demand - instance_->getVehicleCapacity();
      if((int)max_demand > min_removed_demand && (int)min_demand < max_removed_demand){
        i = (last_vehicle_start + 1) % data.size();
        while(!isDepot(data[i])){
          if(nodes[data[i]].demand > min_removed_demand && nodes[data[i]].demand < max_removed_demand){
            const uint prev_node = data[(i - 1 + data.size()) % data.size()];
            const uint next_node = data[(i+1) % data.size()];
            const uint node = data[i];
            int cost = instance_->getDistance(prev_node, kicked_node) + instance_->getDistance(kicked_node, next_node);
            cost += instance_->getDistance(prev_kicked, node) + instance_->getDistance(node, next_kicked);
            cost -= instance_->getDistance(prev_node, node) + instance_->getDistance(node, next_node);
            cost -= instance_->getDistance(prev_kicked, kicked_node) + instance_->getDistance(kicked_node, next_kicked);
            if(cost < best_cost){
              best_cost = cost;
              best_idx = i;
            }
          }
          i = (i + 1) % data.size();
        }
      }
      vehicle_demand = 0;
      min_demand = instance_->getVehicleCapacity();
      max_demand = 0;
      last_vehicle_start = i;
    }
  }
  return best_idx;
}

inline bool CvrpMutationRandom::isCustomer(const uint &node) { return node > 0 && node < (uint)instance_->getNodesCount();}
inline bool CvrpMutationRandom::isDepot(const uint &node) { return !isCustomer(node); }
double CvrpMutationRandom::getMutationRate() { return mutation_rate_; }
void CvrpMutationRandom::setMutationRate(double mutation_rate) { mutation_rate_ = mutation_rate; }

void CvrpMutationRandom::mutateExchangeDemandViolated(
    std::shared_ptr<CvrpIndividual> &individual) {
  auto &data = individual->data();
  uint kicked_idx = selectIdxToKick(individual);

  int insert_idx = selectIdxToExchangeDemandViolated(individual, kicked_idx);
  if(insert_idx == -1)
    return;
  uint kicked_node = data[kicked_idx];
  uint exchange_node = data[insert_idx];
  assert(isCustomer(kicked_node));
  assert(isCustomer(exchange_node));

  uint temp = data[kicked_idx];
  data[kicked_idx] = data[insert_idx];
  data[insert_idx] = temp;

  //assert(individual->assertData());
  individual->resetEvaluated();
  individual->evaluate();
}

int CvrpMutationRandom::selectIdxToExchangeDemandViolated(
    std::shared_ptr<CvrpIndividual> &individual, uint kicked_idx) {
  const auto &nodes = instance_->getNodes();
  auto &data = individual->data();
  const uint kicked_node = data[kicked_idx];
  const uint kicked_demand = nodes[kicked_node].demand;
  const uint prev_kicked = data[(kicked_idx - 1 + data.size()) % data.size()];
  const uint next_kicked = data[(kicked_idx+1) % data.size()];
  assert(isCustomer(kicked_node));
  uint i = kicked_idx;

  while(!isDepot(data[i])){
    i = (i - 1 + data.size()) % data.size();
  }
  uint kicked_vehicle_start = i;
  uint kicked_vehicle_demand = 0;
  i = (i+1) % data.size();
  while(!isDepot(data[i])){
    kicked_vehicle_demand += nodes[data[i]].demand;
    i = (i+1) % data.size();
  }
  uint kicked_vehicle_end = i;

  uint last_vehicle_start = i;
  uint best_demand_violation = individual->getTotalConstraintViolation();
  int best_cost = std::numeric_limits<int>::max();
  int best_idx = -1;
  uint min_demand = instance_->getVehicleCapacity();
  uint max_demand = 0;
  uint vehicle_demand = 0;
  while(i != kicked_vehicle_start){
    i = (i+1) % data.size();
    if(!isDepot(data[i])){
      vehicle_demand += nodes[data[i]].demand;
      if(nodes[data[i]].demand > (int)max_demand)
        max_demand = nodes[data[i]].demand;
      if(nodes[data[i]].demand < (int)min_demand)
        min_demand = nodes[data[i]].demand;
      if(kicked_idx == i){
        vehicle_demand += instance_->getVehicleCapacity();
      }
    }
    else{ // end of vehicle
      uint new_demand_violation = (uint)individual->getTotalConstraintViolation();
      new_demand_violation -= std::max(0, (int)kicked_vehicle_demand - instance_->getVehicleCapacity());
      new_demand_violation -= std::max(0, (int)vehicle_demand - instance_->getVehicleCapacity());
      uint min_new_demand_violation = new_demand_violation +
                                     std::max(0u, vehicle_demand - max_demand + kicked_demand) +
                                     std::max(0u, kicked_vehicle_demand - kicked_demand + min_demand);
      if(min_new_demand_violation < best_demand_violation){
        i = (last_vehicle_start + 1) % data.size();
        while(!isDepot(data[i])){
          const uint node = data[i];
          uint node_demand_violation = new_demand_violation +
                                       std::max(0u, vehicle_demand - nodes[node].demand + kicked_demand) +
                                       std::max(0u, kicked_vehicle_demand - kicked_demand + nodes[node].demand);
          if(node_demand_violation <= best_demand_violation){
            const uint prev_node = data[(i - 1 + data.size()) % data.size()];
            const uint next_node = data[(i+1) % data.size()];
            int cost = instance_->getDistance(prev_node, kicked_node) + instance_->getDistance(kicked_node, next_node);
            cost += instance_->getDistance(prev_kicked, node) + instance_->getDistance(node, next_kicked);
            cost -= instance_->getDistance(prev_node, node) + instance_->getDistance(node, next_node);
            cost -= instance_->getDistance(prev_kicked, kicked_node) + instance_->getDistance(kicked_node, next_kicked);
            if(node_demand_violation < best_demand_violation || cost < best_cost){
              best_cost = cost;
              best_demand_violation = node_demand_violation;
              best_idx = i;
            }
          }
          i = (i + 1) % data.size();
        }
      }
      vehicle_demand = 0;
      min_demand = instance_->getVehicleCapacity();
      max_demand = 0;
      last_vehicle_start = i;
    }
  }
  return best_idx;
}

bool CvrpMutationRandom::mutate2opt(
    std::shared_ptr<CvrpIndividual> &individual) {
  auto new_individual_ = individual->deepcopy();
  auto new_individual = std::static_pointer_cast<CvrpIndividual>(new_individual_);
  auto &data = new_individual->data();
  std::uniform_int_distribution<uint> dist(0, data.size() - 1);

  const uint start = dist(gen);
  const uint length = dist(gen);
  if(length == data.size() - 1)
    return true;
  const uint end = (start + length) % data.size();

  for(uint i = 0; i <= length / 2; i++){
    uint temp = data[(start + i) % data.size()];
    data[(start + i) % data.size()] = data[(end + data.size() - i) % data.size()];
    data[(end + data.size() - i) % data.size()] = temp;
  }

  new_individual->resetEvaluated();
  new_individual->evaluate();
  if(new_individual->betterThan(individual)){
    individual->data() = new_individual->data();
    individual->resetEvaluated();
    individual->evaluate();
  }

  return true;
}
