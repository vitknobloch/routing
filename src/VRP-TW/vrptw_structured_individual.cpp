//
// Created by knoblvit on 1.4.25.
//
#include "VRP-TW/vrptw_structured_individual.h"
#include <algorithm>
#include <cassert>
#include <random>

VrptwIndividualStructured::VrptwIndividualStructured(
    const RoutingInstance *const instance) : instance_(instance), routes_(instance->getVehicleCount()), total_time_(0), total_travel_time_(0), objective_(0), violations_(2, 0), is_evaluated_(false) {}

VrptwIndividualStructured::VrptwIndividualStructured(
    const RoutingInstance *const instance,
    const std::shared_ptr<Solution> &solution) : VrptwIndividualStructured(instance) {
  const auto &nodes = instance->getNodes();
  uint r = 0;
  for(const auto &route: solution->routes){
    routes_[r].time = route.end_time;
    routes_[r].demand = route.demand;
    routes_[r].travel_time = route.travel_time;
    routes_[r].customers.reserve(route.route_nodes.size() - 2);
    routes_[r].time_violation = 0;

    uint time = 0;
    uint travel_time = 0;
    uint demand = 0;
    uint prev_node = 0;
    for(const auto customer: route.route_nodes){
      if(customer.idx == 0){
        continue;
      }
      demand += nodes[customer.idx].demand;
      time += instance->getDistance(prev_node, customer.idx);
      if(time > (uint)nodes[customer.idx].due_date){
        //infeasible due to time constraints
        routes_[r].time_violation += time - (uint)nodes[customer.idx].due_date;
      }
      time = std::max(time, (uint)nodes[customer.idx].ready_time);
      travel_time += instance->getDistance(prev_node, customer.idx);

      routes_[r].customers.emplace_back(customer.idx, travel_time, time, demand);
      time += nodes[customer.idx].service_time;
      prev_node = customer.idx;
    }
    time += instance->getDistance(prev_node, 0);
    travel_time += instance->getDistance(prev_node, 0);
    assert(time == (uint)route.end_time);
    assert(travel_time == (uint)route.travel_time);
    total_time_ += time;
    total_travel_time_ += travel_time;
    timeViolation() += routes_[r].time_violation;
    capacityViolation() += std::max(0, (int)routes_[r].demand - instance->getVehicleCapacity());
    r++;
  }

  is_evaluated_ = true;
}

VrptwIndividualStructured::VrptwIndividualStructured(
    const VrptwIndividualStructured &cpy) : instance_(cpy.instance_), routes_(cpy.routes_), total_time_(cpy.total_time_), violations_(cpy.violations_), is_evaluated_(cpy.is_evaluated_) {}
std::shared_ptr<Solution> VrptwIndividualStructured::convertSolution() {
  evaluate();
  auto solution = std::make_shared<Solution>();
  solution->travel_time_sum = (int)total_travel_time_;
  solution->end_time_sum = (int)total_time_;
  solution->objective = solution->travel_time_sum;

  SolutionNode first_node(0, 0, 0);
  const auto &nodes = instance_->getNodes();

  for(int r = 0; r < (int)routes_.size() - 1; r++){
    const auto &route = routes_[r];
    solution->routes.emplace_back();
    auto &sol_route = solution->routes.back();
    sol_route.demand = (int)route.demand;
    sol_route.travel_time = (int)route.travel_time;
    sol_route.end_time = (int)route.time;
    sol_route.route_nodes.push_back(first_node);

    uint prev_node = 0;
    for(uint i = 0; i < route.customers.size(); i++){
      const auto &customer = route.customers[i];
      sol_route.route_nodes.emplace_back(customer.idx, customer.time_up_to, customer.time_up_to + nodes[customer.idx].service_time);
      sol_route.travel_time += instance_->getDistance(prev_node, customer.idx);
      prev_node = customer.idx;
    }

    sol_route.route_nodes.emplace_back(0, sol_route.end_time, sol_route.end_time);
    sol_route.travel_time += instance_->getDistance(prev_node, 0);
    solution->travel_time_sum += sol_route.travel_time;
    solution->end_time_sum += sol_route.end_time;
  }

  solution->feasible = timeViolation() == 0 && capacityViolation() == 0;

  return solution;
}

void VrptwIndividualStructured::initialize() {
  routes_ = std::vector<VrptwIndividualRoute>(instance_->getVehicleCount());
  const auto &nodes = instance_->getNodes();

  // randomly shuffle customers
  std::random_device rand;
  std::mt19937 gen(rand());
  const uint nodes_count = instance_->getNodesCount();
  std::vector<uint> customers(nodes_count - 1, 0);
  for(int i = 0; i < customers.size(); i++){
    customers[i] = i+1;
  }
  std::shuffle(customers.begin(), customers.end(), gen);

  // add customers to routes if they don't violate demand violation in the order of the shuffle
  uint first_unassigned_customer = 0;
  for(int r = 0; r < instance_->getVehicleCount(); r++){
    for(uint c = first_unassigned_customer; c < customers.size(); c++){
      if(customers[c] == 0)
        continue; //already assigned
      if(routes_[r].demand + nodes[customers[c]].demand > (uint)instance_->getVehicleCapacity() && (uint)r != routes_.size() - 1)
        continue; //violates capacity constraint and is not the last vehicle

      uint prev_node = routes_[r].customers.empty() ? 0 : routes_[r].customers.back().idx;
      const uint time_after = routes_[r].time + instance_->getDistance(prev_node, customers[c]);
      if(time_after > (uint)nodes[customers[c]].due_date){
        if((uint)r != routes_.size() - 1)
          continue; // violates time constraints and is not the last vehicle
        routes_[r].time_violation += time_after - nodes[customers[c]].due_date;
      }

      routes_[r].time = std::max(time_after, (uint)nodes[customers[c]].ready_time) + (uint)nodes[customers[c]].service_time;
      routes_[r].travel_time += instance_->getDistance(prev_node, customers[c]);
      routes_[r].demand += nodes[customers[c]].demand;
      routes_[r].customers.emplace_back(customers[c], routes_[r].travel_time, routes_[r].time, routes_[r].demand);
      customers[c] = 0;
      if(first_unassigned_customer == c)
        first_unassigned_customer++;
    }
    uint prev_node = routes_[r].customers.empty() ? 0 : routes_[r].customers.back().idx;
    routes_[r].time += instance_->getDistance(prev_node, 0);
  }

  // calculate final stats for the whole solution
  capacityViolation() = 0;
  timeViolation() = 0;
  total_time_ = 0;
  for(uint r = 0; r < routes_.size(); r++){
    total_time_ += routes_[r].time;
    total_travel_time_ += routes_[r].travel_time;
    capacityViolation() += std::max(0, (int)routes_[r].demand - instance_->getVehicleCapacity());
    timeViolation() += routes_[r].time_violation;
  }
  is_evaluated_ = true;
}

void VrptwIndividualStructured::evaluateRoute(uint route_idx) {
  auto &route = routes_[route_idx];
  const auto &nodes = instance_->getNodes();

  route.time = 0;
  route.demand = 0;
  route.travel_time = 0;
  route.time_violation = 0;
  uint prev_node = 0;
  for(uint c = 0; c < route.customers.size(); c++){
    auto &customer = route.customers[c];
    const uint dist = instance_->getDistance(prev_node, customer.idx);
    route.time = std::max(route.time + dist, (uint)nodes[customer.idx].ready_time);
    route.travel_time += dist;
    route.demand += (uint)nodes[customer.idx].demand;

    if(route.time > (uint)nodes[customer.idx].due_date){
      route.time_violation += route.time - (uint)nodes[customer.idx].due_date;
    }
    route.time += (uint)nodes[customer.idx].service_time;

    prev_node = customer.idx;
  }

  const uint dist = instance_->getDistance(prev_node, 0);
  route.time += dist;
  route.travel_time += dist;
}
void VrptwIndividualStructured::smartInitialize() { initialize(); }
void VrptwIndividualStructured::resetEvaluated() { is_evaluated_ = false; }
bool VrptwIndividualStructured::betterThan(
    const std::shared_ptr<Individual> &other) {
  if(!other)
    return true;
  auto individual_ = std::static_pointer_cast<VrptwIndividualStructured>(other);
  if(getTotalConstraintViolation() < other->getTotalConstraintViolation())
    return true;
  else if(getTotalConstraintViolation() == other->getTotalConstraintViolation() && total_travel_time_ < other->getFitness())
    return true;
  return false;
}
double VrptwIndividualStructured::getFitness() { return total_travel_time_;}
void VrptwIndividualStructured::calculateConstraints() {
    evaluate();
}
void VrptwIndividualStructured::calculateFitness() {
  evaluate();
}
const std::vector<double> &VrptwIndividualStructured::getConstraintViolations(){
    return violations_;
}
double VrptwIndividualStructured::getTotalConstraintViolation() {
    return violations_[0] + violations_[1];
}
bool VrptwIndividualStructured::isEvaluated() { return is_evaluated_;}
std::shared_ptr<Individual> VrptwIndividualStructured::deepcopy() {
    return std::make_shared<VrptwIndividualStructured>(*this);
}

void VrptwIndividualStructured::evaluate() {
  total_time_ = 0;
  total_travel_time_ = 0;
  timeViolation() = 0;
  capacityViolation() = 0;

  for(uint r = 0; r < routes_.size(); r++){
    evaluateRoute(r);
    total_time_ += routes_[r].time;
    total_travel_time_ += routes_[r].travel_time;
    capacityViolation() += std::max(0, (int)routes_[r].demand - instance_->getVehicleCapacity());
    timeViolation() += routes_[r].time_violation;
  }
}
const std::vector<VrptwIndividualRoute> &
VrptwIndividualStructured::getRoutes() {
  return routes_;
}
bool VrptwIndividualStructured::test2optMove(const VrptwRouteSegment &segment) {
  if(segment.segment_length < 2)
    return false;

  // find out if the traveled distance after exchange will be lower than before
  const uint end_idx = segment.segment_start_idx + segment.segment_length - 1;
  assert(segment.route_idx < routes_.size());
  const auto &customers = routes_[segment.route_idx].customers;
  assert(segment.segment_start_idx < customers.size() && end_idx < customers.size());
  const uint pre_node = segment.segment_start_idx == 0 ? 0 : customers[segment.segment_start_idx - 1].idx;
  const uint post_node = end_idx + 1 >= customers.size() ? 0 : customers[end_idx + 1].idx;
  const uint first_node = customers[segment.segment_start_idx].idx;
  const uint last_node = customers[end_idx].idx;

  const uint old_length = instance_->getDistance(pre_node, first_node) + instance_->getDistance(last_node, post_node);
  const uint new_length = instance_->getDistance(pre_node, last_node) + instance_->getDistance(first_node, post_node);
  if(old_length <= new_length)
    return false;

  //Find time violation of the exchanged segment
  const auto &nodes = instance_->getNodes();
  uint time = segment.segment_start_idx == 0 ? 0 : customers[segment.segment_start_idx - 1].time_up_to + nodes[pre_node].service_time;
  uint prev_node = pre_node;
  uint time_violation = 0;
  for(uint c = end_idx; c >= segment.segment_start_idx; c--){
    time += instance_->getDistance(prev_node, customers[c].idx);
    if(time > (uint)nodes[customers[c].idx].due_date){
      time_violation += time - nodes[customers[c].idx].due_date;
    }
    time = std::max(time, (uint)nodes[customers[c].idx].ready_time);
    time += nodes[customers[c].idx].service_time;
    prev_node += customers[c].idx;
  }
  time += instance_->getDistance(prev_node, post_node);

  const uint old_time_after = customers[end_idx].time_up_to + instance_->getDistance(last_node, post_node);
  if(time_violation == 0 && time <= old_time_after)
    return true; // no additional time violation here nor in the future

  // find time violation of the original segment
  uint old_time_violation = 0;
  for(uint c = segment.segment_start_idx; c < segment.segment_start_idx + segment.segment_length; c++){
    old_time_violation += std::max(0, (int)customers[c].time_up_to - nodes[customers[c].idx].due_date);
  }

  if(time <= old_time_after && time_violation <= old_time_violation)
    return true;
  uint shift = time - old_time_after;
  uint c = end_idx + 1;
  while(c < customers.size() && shift > 0){
    if(customers[c].time_up_to + shift > (uint)nodes[customers[c].idx].due_date){
      time_violation += customers[c].time_up_to + shift - (uint)nodes[customers[c].idx].due_date;
      old_time_violation += std::max(0, (int)customers[c].time_up_to - nodes[customers[c].idx].due_date);
    }
    //TODO: change shift if the customer is visited before ready_time
  }

}
