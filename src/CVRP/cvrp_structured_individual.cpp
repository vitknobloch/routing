//
// Created by knoblvit on 30.3.25.
//
#include "CVRP/cvrp_structured_individual.h"
#include <cassert>
#include <random>
#include <algorithm>

CvrpIndividualStructured::CvrpIndividualStructured(
    const RoutingInstance *const instance) : instance_(instance), routes_(instance->getVehicleCount()), total_time_(0), demand_violation_(1, 0), is_evaluated_(false) {}
CvrpIndividualStructured::CvrpIndividualStructured(
    const RoutingInstance *const instance,
    const std::shared_ptr<Solution> &solution) : instance_(instance), routes_(instance->getVehicleCount()), total_time_(0), demand_violation_(1, 0), is_evaluated_(false) {
  const auto &nodes = instance->getNodes();
  uint r = 0;
  for(const auto &route: solution->routes){
    routes_[r].time = route.travel_time;
    routes_[r].demand = route.demand;
    routes_[r].customers.reserve(route.route_nodes.size() - 2);

    uint time = 0;
    uint demand = 0;
    uint prev_node = 0;
    for(const auto customer: route.route_nodes){
      if(customer.idx == 0){
        continue;
      }
      time += instance->getDistance(prev_node, customer.idx);
      demand += nodes[customer.idx].demand;
      routes_[r].customers.emplace_back(customer.idx, time, demand);
      prev_node = customer.idx;
    }
    time += instance->getDistance(prev_node, 0);
    assert(time == (uint)route.travel_time);
    assert(demand == (uint)route.demand);
    total_time_ += time;
    demand_violation_[0] += std::max(0, (int)demand - instance->getVehicleCapacity());
    r++;
  }
  assert(total_time_ == solution->travel_time_sum);
  is_evaluated_ = true;
}

CvrpIndividualStructured::CvrpIndividualStructured(
    const CvrpIndividualStructured &cpy) : instance_(cpy.instance_), routes_(cpy.routes_), total_time_(cpy.total_time_), demand_violation_(cpy.demand_violation_), is_evaluated_(cpy.is_evaluated_) {}

void CvrpIndividualStructured::initialize() {
  routes_ = std::vector<CvrpIndividualRoute>(instance_->getVehicleCount());
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
        continue; //violates constraints and is not the last vehicle
      routes_[r].demand += nodes[customers[c]].demand;
      uint prev_node = routes_[r].customers.empty() ? 0 : routes_[r].customers.back().idx;
      routes_[r].time += instance_->getDistance(prev_node, customers[c]);
      routes_[r].customers.emplace_back(customers[c], routes_[r].time, routes_[r].demand);
      customers[c] = 0;
      if(first_unassigned_customer == c)
        first_unassigned_customer++;
    }
    uint prev_node = routes_[r].customers.empty() ? 0 : routes_[r].customers.back().idx;
    routes_[r].time += instance_->getDistance(prev_node, 0);
  }

  // calculate final stats for the whole solution
  demand_violation_[0] = 0;
  total_time_ = 0;
  for(uint r = 0; r < routes_.size(); r++){
    total_time_ += routes_[r].time;
    demand_violation_[0] += std::max(0, (int)routes_[r].demand - instance_->getVehicleCapacity());
  }
  is_evaluated_ = true;
}

void CvrpIndividualStructured::resetEvaluated() {
  is_evaluated_ = false;
}

bool CvrpIndividualStructured::betterThan(
    const std::shared_ptr<Individual> &other) {
  if(!other)
    return true;
  auto individual_ = std::static_pointer_cast<CvrpIndividualStructured>(other);
  if(demand_violation_[0] < other->getTotalConstraintViolation())
    return true;
  else if(demand_violation_[0] == other->getTotalConstraintViolation() && total_time_ < other->getFitness())
    return true;
  return false;
}

double CvrpIndividualStructured::getFitness() { return total_time_; }

void CvrpIndividualStructured::calculateConstraints() {
  evaluate();
}

void CvrpIndividualStructured::calculateFitness() {
  evaluate();
}

const std::vector<double> &CvrpIndividualStructured::getConstraintViolations() {
  return demand_violation_;
}

double CvrpIndividualStructured::getTotalConstraintViolation() { return demand_violation_[0]; }

bool CvrpIndividualStructured::isEvaluated() { return is_evaluated_; }

std::shared_ptr<Individual> CvrpIndividualStructured::deepcopy() {
  return std::make_shared<CvrpIndividualStructured>(*this);
}

void CvrpIndividualStructured::evaluate() {
  if(is_evaluated_)
    return;

  const auto &nodes = instance_->getNodes();
  total_time_ = 0;
  demand_violation_[0] = 0;
  for(uint r = 0; r < routes_.size(); r++){
    auto &route = routes_[r];
    route.demand = 0;
    route.time = 0;
    uint prev_node = 0;
    auto customers = route.customers;
    for(uint c = 0; c < customers.size(); c++){
      uint cur_node = customers[c].idx;
      route.demand += nodes[cur_node].demand;
      route.time += instance_->getDistance(prev_node, cur_node);
      customers[c].time_up_to = route.time;
      customers[c].demand_up_to = route.demand;
      prev_node = cur_node;
    }
    route.time += instance_->getDistance(prev_node, 0);
  }
}
const std::vector<CvrpIndividualRoute> &CvrpIndividualStructured::getRoutes() {
  return routes_;
}

void CvrpIndividualStructured::perform2optMove(
    const CvrpRouteSegment &segment) {
  if(segment.segment_length < 2)
    return;
  assert(segment.route_idx < routes_.size());
  auto &route = routes_[segment.route_idx];
  assert(segment.segment_start_idx < route.customers.size());
  assert(segment.segment_start_idx + segment.segment_length <= route.customers.size());

  uint left = segment.segment_start_idx;
  uint right = segment.segment_start_idx + segment.segment_length - 1;
  while(left < right){
    std::swap(route.customers[left].idx, route.customers[right].idx);
    left++;
    right--;
  }

  uint prev_node = 0;
  uint time = 0;
  uint demand = 0;
  if(segment.segment_start_idx > 0){
    const auto &prev_customer = route.customers[segment.segment_start_idx - 1];
    prev_node = prev_customer.idx;
    time = prev_customer.time_up_to;
    demand = prev_customer.demand_up_to;
  }
  const auto &nodes = instance_->getNodes();
  for(uint i = segment.segment_start_idx; i < route.customers.size(); i++){
    const uint cur_node = route.customers[i].idx;
    time += instance_->getDistance(prev_node, cur_node);
    demand += nodes[cur_node].demand;
    route.customers[i].time_up_to = time;
    route.customers[i].demand_up_to = demand;
    prev_node = cur_node;
  }
  time += instance_->getDistance(prev_node, 0);
  total_time_ -= route.time;
  total_time_ += time;
  route.time = time;
}

void CvrpIndividualStructured::performExchangeMove(
    const CvrpRouteSegment &segment1, const CvrpRouteSegment &segment2) {
  assert(segment1.route_idx < routes_.size() && segment2.route_idx < routes_.size());
  const auto &route1 = routes_[segment1.route_idx];
  const auto &route2 = routes_[segment2.route_idx];

  assert(segment1.segment_start_idx < route1.customers.size() && segment1.segment_start_idx + segment1.segment_length <= route1.customers.size());
  assert(segment2.segment_start_idx < route2.customers.size() && segment2.segment_start_idx + segment2.segment_length <= route2.customers.size());

  auto new_route1 = insertSegment(route1, route2, segment1, segment2);
  auto new_route2 = insertSegment(route2, route1, segment2, segment1);

  exchange_route(segment1.route_idx, new_route1);
  exchange_route(segment2.route_idx, new_route2);
}

CvrpIndividualRoute CvrpIndividualStructured::insertSegment(
    const CvrpIndividualRoute &to_route, const CvrpIndividualRoute &from_route,
    const CvrpRouteSegment &remove_segment,
    const CvrpRouteSegment &insert_segment) {
  const auto &nodes = instance_->getNodes();

  CvrpIndividualRoute new_route;
  new_route.customers.reserve(to_route.customers.size() -
                              remove_segment.segment_length +
                              insert_segment.segment_length);
  new_route.time = 0;
  new_route.demand = 0;
  uint prev_node = 0;
  // insert nodes before removed segment
  for(int i = 0; i < remove_segment.segment_start_idx; i++){
    new_route.customers.push_back(to_route.customers[i]);
  }
  if(!new_route.customers.empty()){
    const auto &prev_customer = new_route.customers.back();
    new_route.time = prev_customer.time_up_to;
    new_route.demand = prev_customer.demand_up_to;
    prev_node = prev_customer.idx;
  }

  // insert nodes from inserted segment
  for(int i = insert_segment.segment_start_idx; i < insert_segment.segment_start_idx + insert_segment.segment_length; i++){
    const uint cur_node = from_route.customers[i].idx;
    new_route.time += instance_->getDistance(prev_node, cur_node);
    new_route.demand += nodes[cur_node].demand;
    new_route.customers.emplace_back(cur_node, new_route.time,
                                     new_route.demand);
    prev_node = cur_node;
  }

  // insert nodes after removed segment
  for(int i = remove_segment.segment_start_idx + remove_segment.segment_length; i < to_route.customers.size(); i++){
    const uint cur_node = to_route.customers[i].idx;
    new_route.time += instance_->getDistance(prev_node, cur_node);
    new_route.demand += nodes[cur_node].demand;
    new_route.customers.emplace_back(cur_node, new_route.time, new_route.demand);
    prev_node = cur_node;
  }
  new_route.time += instance_->getDistance(prev_node, 0);
  return new_route;
}

void CvrpIndividualStructured::performRelocateMove(
    const CvrpRouteSegment &segment_moved, const CvrpRouteSegment &target_pos) {
  assert(segment_moved.route_idx < routes_.size() && target_pos.route_idx < routes_.size());
  const auto &route_from = routes_[segment_moved.route_idx];
  const auto &route_to = routes_[target_pos.route_idx];

  assert(segment_moved.segment_start_idx < route_from.customers.size() && segment_moved.segment_start_idx + segment_moved.segment_length <= route_from.customers.size());
  assert(target_pos.segment_start_idx <= route_to.customers.size());
  assert(target_pos.segment_length == 0);

  auto new_route_from = insertSegment(route_from, route_to, segment_moved, target_pos);
  auto new_route_to = insertSegment(route_to, route_from, target_pos, segment_moved);

  exchange_route(segment_moved.route_idx, new_route_from);
  exchange_route(target_pos.route_idx, new_route_to);
}

void CvrpIndividualStructured::performCrossMove(
    const CvrpRouteSegment &segment1, const CvrpRouteSegment &segment2) {
  assert(segment1.route_idx < routes_.size() && segment2.route_idx < routes_.size());
  const auto &route1 = routes_[segment1.route_idx];
  const auto &route2 = routes_[segment2.route_idx];

  assert(segment1.segment_start_idx <= route1.customers.size());
  assert(segment2.segment_start_idx <= route2.customers.size());

  CvrpRouteSegment segment1_ = segment1;
  CvrpRouteSegment segment2_ = segment2;
  segment1_.segment_length = route1.customers.size() - segment1.segment_start_idx;
  segment2_.segment_length = route2.customers.size() - segment2.segment_start_idx;
  auto new_route1 = insertSegment(route1, route2, segment1_, segment2_);
  auto new_route2 = insertSegment(route2, route1, segment2_, segment1_);

  exchange_route(segment1.route_idx, new_route1);
  exchange_route(segment2.route_idx, new_route2);

}

void CvrpIndividualStructured::exchange_route(
    const uint &route_idx, const CvrpIndividualRoute &new_route) {
  const auto &old_route = routes_[route_idx];

  total_time_ -= old_route.time;
  total_time_ += new_route.time;
  const auto &capacity = instance_->getVehicleCapacity();
  demand_violation_[0] -= std::max(0, (int)old_route.demand - capacity);
  demand_violation_[0] += std::max(0, (int)new_route.demand - capacity);
  routes_[route_idx] = std::move(new_route);
}

uint CvrpIndividualStructured::getSegmentTime(const CvrpRouteSegment &segment) {
  if(segment.segment_length == 0)
    return 0;
  auto start_time = routes_[segment.route_idx].customers[segment.segment_start_idx].time_up_to;
  auto end_time = routes_[segment.route_idx].customers[segment.segment_start_idx + segment.segment_length - 1].time_up_to;
  return end_time - start_time;
}

uint CvrpIndividualStructured::getSegmentDemand(
    const CvrpRouteSegment &segment) {
  if(segment.segment_length == 0)
    return 0;
  auto demand_before = segment.segment_start_idx == 0 ? 0 : routes_[segment.route_idx].customers[segment.segment_start_idx - 1].demand_up_to;
  auto demand_after = routes_[segment.route_idx].customers[segment.segment_start_idx + segment.segment_length - 1].demand_up_to;
  return demand_after - demand_before;
}

uint CvrpIndividualStructured::getExchangeTime(
    const CvrpRouteSegment &remove_segment,
    const CvrpRouteSegment &insert_segment) {
  const auto &cust_to = routes_[remove_segment.route_idx].customers;
  const auto &cust_from = routes_[insert_segment.route_idx].customers;
  uint prev_node = 0;
  uint prev_time = 0;
  if(remove_segment.segment_start_idx > 0){
    prev_node = cust_to[remove_segment.segment_start_idx - 1].idx;
    prev_time = cust_to[remove_segment.segment_start_idx - 1].time_up_to;
  }
  uint next_node = 0;
  uint after_time = 0;
  const uint after_idx = remove_segment.segment_start_idx + remove_segment.segment_length;
  if(after_idx < cust_to.size()){
    next_node = cust_to[after_idx].idx;
    after_time = routes_[remove_segment.route_idx].time - cust_to[after_idx].time_up_to;
  }

  if(insert_segment.segment_length == 0){
    return prev_time + instance_->getDistance(prev_node, next_node) + after_time;
  }
  const uint first_node = cust_from[insert_segment.segment_start_idx].idx;
  const uint last_node = cust_from[insert_segment.segment_start_idx +
                                        insert_segment.segment_length - 1].idx;

  return prev_time + instance_->getDistance(prev_node, first_node) + getSegmentTime(insert_segment) + instance_->getDistance(last_node, next_node) + after_time;
}

bool CvrpIndividualStructured::test2optMove(const CvrpRouteSegment &segment) {
  if(segment.segment_length < 2)
    return false;
  const uint end_idx = segment.segment_start_idx + segment.segment_length - 1;
  assert(segment.route_idx < routes_.size());
  const auto &customers = routes_[segment.route_idx].customers;
  assert(segment.segment_start_idx < customers.size() && end_idx < customers.size());
  const uint prev_node = segment.segment_start_idx == 0 ? 0 : customers[segment.segment_start_idx - 1].idx;
  const uint next_node = end_idx + 1 >= customers.size() ? 0 : customers[end_idx + 1].idx;
  const uint first_node = customers[segment.segment_start_idx].idx;
  const uint last_node = customers[end_idx].idx;

  const uint old_length = instance_->getDistance(prev_node, first_node) + instance_->getDistance(last_node, next_node);
  const uint new_length = instance_->getDistance(prev_node, last_node) + instance_->getDistance(first_node, next_node);
  return new_length < old_length;
}

bool CvrpIndividualStructured::testExchangeMove(
    const CvrpRouteSegment &segment1, const CvrpRouteSegment &segment2) {
  assert(segment1.route_idx != segment2.route_idx);
  assert(segment1.route_idx < routes_.size() && segment2.route_idx < routes_.size());
  const auto &route1 = routes_[segment1.route_idx];
  const auto &route2 = routes_[segment2.route_idx];
  assert(segment1.segment_start_idx <= route1.customers.size() && segment1.segment_start_idx + segment1.segment_length <= route1.customers.size());
  assert(segment2.segment_start_idx <= route2.customers.size() && segment2.segment_start_idx + segment2.segment_length <= route2.customers.size());
  const uint demand1 = getSegmentDemand(segment1);
  const uint demand2 = getSegmentDemand(segment2);

  const auto &capacity = instance_->getVehicleCapacity();
  const int demand_violation_old = std::max(0, (int)route1.demand - capacity) +
                                   std::max(0, (int)route2.demand - capacity);

  const int demand_violation_new = std::max(0, (int)route1.demand + (int)demand2 - (int)demand1 - capacity) +
                                   std::max(0, (int)route2.demand + (int)demand1 - (int)demand2 - capacity);

  if(demand_violation_new > demand_violation_old)
    return false;

  const uint time_old = route1.time + route2.time;
  const uint time_new = getExchangeTime(segment1, segment2) + getExchangeTime(segment2, segment1);
  return time_new < time_old;
}

bool CvrpIndividualStructured::testRelocateMove(
    const CvrpRouteSegment &segment_moved, const CvrpRouteSegment &target_pos) {
  assert(segment_moved.route_idx != target_pos.route_idx);
  assert(segment_moved.route_idx < routes_.size() && target_pos.route_idx < routes_.size());
  const auto &route_from = routes_[segment_moved.route_idx];
  const auto &route_to = routes_[target_pos.route_idx];
  assert(segment_moved.segment_start_idx < route_from.customers.size() && segment_moved.segment_start_idx + segment_moved.segment_length <= route_from.customers.size());
  assert(target_pos.segment_start_idx <= route_to.customers.size() && target_pos.segment_length == 0);
  const uint demand = getSegmentDemand(segment_moved);

  const auto &capacity = instance_->getVehicleCapacity();
  const int demand_violation_old = std::max(0, (int)route_from.demand - capacity) +
                                   std::max(0, (int)route_to.demand - capacity);

  const int demand_violation_new = std::max(0, (int)route_from.demand - (int)demand - capacity) +
                                   std::max(0, (int)route_to.demand + (int)demand - capacity);

  if(demand_violation_new > demand_violation_old)
    return false;

  const uint time_old = route_from.time + route_to.time;
  const uint time_new = getExchangeTime(segment_moved, target_pos) + getExchangeTime(target_pos, segment_moved);
  return time_new < time_old;
}

bool CvrpIndividualStructured::testCrossMove(const CvrpRouteSegment &segment1,
                                             const CvrpRouteSegment &segment2) {
  auto segment1_ = segment1;
  auto segment2_ = segment2;
  segment1_.segment_length = routes_[segment1.route_idx].customers.size() - segment1.segment_start_idx;
  segment2_.segment_length = routes_[segment2.route_idx].customers.size() - segment2.segment_start_idx;
  return testExchangeMove(segment1_, segment2_);
}

void CvrpIndividualStructured::smartInitialize() { initialize(); }

std::shared_ptr<Solution> CvrpIndividualStructured::convertSolution() {
  evaluate();
  auto solution = std::make_shared<Solution>();
  solution->travel_time_sum = total_time_;
  solution->end_time_sum = total_time_ + (instance_->getNodesCount() - 1) * 1;
  solution->objective = solution->travel_time_sum;

  SolutionNode first_node(0, 0, 0);

  for(const auto &route: routes_){
    solution->routes.emplace_back();
    auto &sol_route = solution->routes.back();
    sol_route.demand = route.demand;
    sol_route.travel_time = route.time;
    sol_route.end_time = route.time + route.customers.size() * 1;
    sol_route.route_nodes.push_back(first_node);

    for(uint i = 0; i < route.customers.size(); i++){
      const auto &customer = route.customers[i];
      sol_route.route_nodes.emplace_back(customer.idx, customer.time_up_to + i, customer.time_up_to + i + 1);
    }
    sol_route.route_nodes.emplace_back(0, sol_route.end_time, sol_route.end_time);
  }

  solution->used_vehicles = (int)routes_.size();
  solution->feasible = getTotalConstraintViolation() == 0;

  return solution;
}
