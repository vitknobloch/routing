#include "VRP-TW/vrptw_structured_individual.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>

VrptwIndividualStructured::VrptwIndividualStructured(
    const RoutingInstance *const instance) : instance_(instance), routes_(instance->getVehicleCount()), total_time_(0), total_travel_time_(0), vehicles_used_(0), violations_(2, 0), is_evaluated_(false) {}

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

  for(const auto &route: routes_)
    if(!route.customers.empty())
      vehicles_used_++;

  is_evaluated_ = true;
}

VrptwIndividualStructured::VrptwIndividualStructured(
    const VrptwIndividualStructured &cpy) : instance_(cpy.instance_), routes_(cpy.routes_), total_time_(cpy.total_time_), total_travel_time_(cpy.total_travel_time_), vehicles_used_(cpy.vehicles_used_), violations_(cpy.violations_), is_evaluated_(cpy.is_evaluated_) {}

VrptwIndividualStructured::VrptwIndividualStructured(
    const VrptwIndividualStructured &cpy, const std::vector<uint> &flat_data) : instance_(cpy.instance_), routes_(instance_->getVehicleCount()), total_time_(0), total_travel_time_(0), vehicles_used_(0), violations_(2, 0), is_evaluated_(false) {
  uint i = 0;
  for(auto & route : routes_){
    route.customers = std::vector<VrptwIndividualCustomer>();
    while(flat_data[i] < instance_->getNodesCount()){
      uint cur_node = flat_data[i];
      route.customers.emplace_back(cur_node, 0, 0, 0);
      i++;
    }
    i++;
  }
  evaluate();
}

std::shared_ptr<Solution> VrptwIndividualStructured::convertSolution() {
  evaluate();
  auto solution = std::make_shared<Solution>();
  solution->travel_time_sum = (int)total_travel_time_;
  solution->end_time_sum = (int)total_time_;
  solution->objective = solution->travel_time_sum;

  SolutionNode first_node(0, 0, 0);
  const auto &nodes = instance_->getNodes();

  for(int r = 0; r < (int)routes_.size(); r++){
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
      //sol_route.travel_time += instance_->getDistance(prev_node, customer.idx);
      prev_node = customer.idx;
    }

    sol_route.route_nodes.emplace_back(0, sol_route.end_time, sol_route.end_time);
    //sol_route.travel_time += instance_->getDistance(prev_node, 0);
    //solution->travel_time_sum += sol_route.travel_time;
    //solution->end_time_sum += sol_route.end_time;
  }

  solution->objective = solution->travel_time_sum;
  solution->feasible = (int)timeViolation() == 0 && (int)capacityViolation() == 0;
  solution->used_vehicles = (int)vehicles_used_;

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
    if(!routes_[r].customers.empty())
      vehicles_used_++;
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
    customer.time_up_to = route.time;
    customer.travel_time_up_to = route.travel_time;
    customer.demand_up_to = route.demand;

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
  if((int)getTotalConstraintViolation() < (int)other->getTotalConstraintViolation())
    return true;
  else if((int)getTotalConstraintViolation() == (int)other->getTotalConstraintViolation()){
    const auto &other_ = std::static_pointer_cast<VrptwIndividualStructured>(other);
    if(vehicles_used_ < other_->vehicles_used_)
      return true;
    else if(vehicles_used_ == other_->vehicles_used_ && getFitness() < other_->getFitness())
      return true;
  }
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
  vehicles_used_ = 0;
  timeViolation() = 0;
  capacityViolation() = 0;

  for(uint r = 0; r < routes_.size(); r++){
    evaluateRoute(r);
    if(!routes_[r].customers.empty())
      vehicles_used_++;
    total_time_ += routes_[r].time;
    total_travel_time_ += routes_[r].travel_time;
    capacityViolation() += std::max(0, (int)routes_[r].demand - instance_->getVehicleCapacity());
    timeViolation() += routes_[r].time_violation;
  }

  is_evaluated_ = true;
}
const std::vector<VrptwIndividualRoute> &
VrptwIndividualStructured::getRoutes() {
  return routes_;
}
bool VrptwIndividualStructured::test2optMove(const VrptwRouteSegment &segment) {
  if(segment.segment_length < 2)
    return false;
  assert(segment.route_idx < routes_.size());
  assert(segment.segment_start_idx < routes_[segment.route_idx].customers.size());
  assert(segment.segment_start_idx + segment.segment_length <= routes_[segment.route_idx].customers.size());
  if(routes_[segment.route_idx].time_violation > 0)
    return test2optMoveViolation(segment);
  else
    return test2optMoveNoViolations(segment);
}

void VrptwIndividualStructured::perform2optMove(
    const VrptwRouteSegment &segment) {
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

  const int prev_time = (int)route.time;
  const int prev_travel_time = (int)route.travel_time;
  const int prev_time_violation = (int)route.time_violation;
  evaluateRoute(segment.route_idx);
  assertEvaluation(route);

  total_time_ += (route.time - prev_time);
  total_travel_time_ += (route.travel_time - prev_travel_time);
  timeViolation() += ((int)route.time_violation - prev_time_violation);
}

VrptwIndividualRoute VrptwIndividualStructured::insertSegment(
    const VrptwIndividualRoute &to_route,
    const VrptwIndividualRoute &from_route,
    const VrptwRouteSegment &remove_segment,
    const VrptwRouteSegment &insert_segment) {
  const auto &nodes = instance_->getNodes();
  VrptwIndividualRoute new_route;
  new_route.customers.reserve(to_route.customers.size() - remove_segment.segment_length + insert_segment.segment_length);
  new_route.time = 0;
  new_route.travel_time = 0;
  new_route.time_violation = 0;
  new_route.demand = 0;
  uint prev_node = 0;
  // Insert nodes before removed segment
  for(uint i = 0; i < (uint)remove_segment.segment_start_idx; i++){
    const uint cur_node = to_route.customers[i].idx;
    new_route.time += instance_->getDistance(prev_node, cur_node);
    new_route.travel_time += instance_->getDistance(prev_node, cur_node);
    if(new_route.time > (uint)nodes[cur_node].due_date)
      new_route.time_violation += new_route.time - (uint)nodes[cur_node].due_date;
    new_route.demand += nodes[cur_node].demand;
    new_route.time = std::max(new_route.time, (uint)nodes[cur_node].ready_time);
    new_route.customers.emplace_back(cur_node, new_route.travel_time, new_route.time,
                                     new_route.demand);
    new_route.time += nodes[cur_node].service_time;
    prev_node = cur_node;
  }

  // Insert nodes from insert segment
  for(uint i = insert_segment.segment_start_idx; i < insert_segment.segment_start_idx + insert_segment.segment_length; i++){
    const uint cur_node = from_route.customers[i].idx;
    new_route.time += instance_->getDistance(prev_node, cur_node);
    new_route.travel_time += instance_->getDistance(prev_node, cur_node);
    if(new_route.time > (uint)nodes[cur_node].due_date)
      new_route.time_violation += new_route.time - (uint)nodes[cur_node].due_date;
    new_route.demand += nodes[cur_node].demand;
    new_route.time = std::max(new_route.time, (uint)nodes[cur_node].ready_time);
    new_route.customers.emplace_back(cur_node, new_route.travel_time, new_route.time,
                                     new_route.demand);
    new_route.time += nodes[cur_node].service_time;
    prev_node = cur_node;
  }

  // Insert nodes after removed segment
  for(uint i = remove_segment.segment_start_idx + remove_segment.segment_length; i < to_route.customers.size(); i++){
    const uint cur_node = to_route.customers[i].idx;
    new_route.time += instance_->getDistance(prev_node, cur_node);
    new_route.travel_time += instance_->getDistance(prev_node, cur_node);
    if(new_route.time > (uint)nodes[cur_node].due_date)
      new_route.time_violation += new_route.time - (uint)nodes[cur_node].due_date;
    new_route.demand += nodes[cur_node].demand;
    new_route.time = std::max(new_route.time, (uint)nodes[cur_node].ready_time);
    new_route.customers.emplace_back(cur_node, new_route.travel_time, new_route.time,
                                     new_route.demand);
    new_route.time += nodes[cur_node].service_time;
    prev_node = cur_node;
  }

  new_route.travel_time += instance_->getDistance(prev_node, 0);
  new_route.time += instance_->getDistance(prev_node, 0);
  return new_route;
}

void VrptwIndividualStructured::exchange_route(
    const uint &route_idx, const VrptwIndividualRoute &new_route) {
  const auto &old_route = routes_[route_idx];

  total_time_ += new_route.time - old_route.time;
  total_travel_time_ += new_route.travel_time - old_route.travel_time;
  const auto &capacity = instance_->getVehicleCapacity();
  capacityViolation() += (std::max(0, (int)new_route.demand - capacity) - std::max(0, (int)old_route.demand - capacity));
  timeViolation() += ((int)new_route.time_violation - (int)old_route.time_violation);
  vehicles_used_ += (old_route.customers.empty() ? 0 : -1) + (new_route.customers.empty() ? 0 : 1);
  routes_[route_idx] = new_route;
}

void VrptwIndividualStructured::performExchangeMove(
    const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2) {
  assert(segment1.route_idx < routes_.size() && segment2.route_idx < routes_.size());
  const auto &route1 = routes_[segment1.route_idx];
  const auto &route2 = routes_[segment2.route_idx];

  assert(segment1.segment_start_idx < route1.customers.size() && segment1.segment_start_idx + segment1.segment_length <= route1.customers.size());
  assert(segment2.segment_start_idx < route2.customers.size() && segment2.segment_start_idx + segment2.segment_length <= route2.customers.size());

  auto new_route1 = insertSegment(route1, route2, segment1, segment2);
  auto new_route2 = insertSegment(route2, route1, segment2, segment1);

  /*
  assert(new_route1.time_violation + new_route2.time_violation <= route1.time_violation + route2.time_violation);
  assertEvaluation(route1);
  assertEvaluation(route2);
  assertEvaluation(new_route1);
  assertEvaluation(new_route2);
  if(route1.time_violation + route2.time_violation == 0){
    assert(route1.travel_time + route2.travel_time > new_route1.travel_time + new_route2.travel_time);
  }
  */

  exchange_route(segment1.route_idx, new_route1);
  exchange_route(segment2.route_idx, new_route2);
}

void VrptwIndividualStructured::performRelocateMove(
    const VrptwRouteSegment &segment_moved,
    const VrptwRouteSegment &target_pos) {
  assert(segment_moved.route_idx < routes_.size() && target_pos.route_idx < routes_.size());
  const auto &route_from = routes_[segment_moved.route_idx];
  const auto &route_to = routes_[target_pos.route_idx];

  assert(segment_moved.segment_start_idx < route_from.customers.size() && segment_moved.segment_start_idx + segment_moved.segment_length <= route_from.customers.size());
  assert(target_pos.segment_start_idx <= route_to.customers.size());
  assert(target_pos.segment_length == 0);

  auto new_route_from = insertSegment(route_from, route_to, segment_moved, target_pos);
  auto new_route_to = insertSegment(route_to, route_from, target_pos, segment_moved);
  /*
  assert(new_route_from.time_violation + new_route_to.time_violation <= route_from.time_violation + route_to.time_violation);
  assertEvaluation(route_from);
  assertEvaluation(route_to);
  assertEvaluation(new_route_from);
  assertEvaluation(new_route_to);
  */

  exchange_route(segment_moved.route_idx, new_route_from);
  exchange_route(target_pos.route_idx, new_route_to);
}

void VrptwIndividualStructured::performCrossMove(
    const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2) {
  assert(segment1.route_idx < routes_.size() && segment2.route_idx < routes_.size());
  const auto &route1 = routes_[segment1.route_idx];
  const auto &route2 = routes_[segment2.route_idx];

  assert(segment1.segment_start_idx <= route1.customers.size());
  assert(segment2.segment_start_idx <= route2.customers.size());

  VrptwRouteSegment segment1_ = segment1;
  VrptwRouteSegment segment2_ = segment2;
  segment1_.segment_length = route1.customers.size() - segment1.segment_start_idx;
  segment2_.segment_length = route2.customers.size() - segment2.segment_start_idx;
  auto new_route1 = insertSegment(route1, route2, segment1_, segment2_);
  auto new_route2 = insertSegment(route2, route1, segment2_, segment1_);

  /*
  assert(route1.time_violation + route2.time_violation >= new_route1.time_violation + new_route2.time_violation);
  assertEvaluation(route1);
  assertEvaluation(route2);
  assertEvaluation(new_route1);
  assertEvaluation(new_route2);*/

  exchange_route(segment1.route_idx, new_route1);
  exchange_route(segment2.route_idx, new_route2);
}

bool VrptwIndividualStructured::testExchangeMove(
    const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2) {
  assert(segment1.route_idx != segment2.route_idx);
  assert(segment1.route_idx < routes_.size() && segment2.route_idx < routes_.size());
  const auto &route1 = routes_[segment1.route_idx];
  const auto &route2 = routes_[segment2.route_idx];
  assert(segment1.segment_start_idx <= route1.customers.size() && segment1.segment_start_idx + segment1.segment_length <= route1.customers.size());
  assert(segment2.segment_start_idx <= route2.customers.size() && segment2.segment_start_idx + segment2.segment_length <= route2.customers.size());
  if(
      route1.time_violation > 0 ||
      route2.time_violation > 0 ||
      route1.demand > (uint)instance_->getVehicleCapacity() ||
      route2.demand > (uint)instance_->getVehicleCapacity()
      )
    return testExchangeMoveViolation(segment1, segment2);
  else
    return testExchangeMoveNoViolation(segment1, segment2);
}

uint VrptwIndividualStructured::getSegmentTravelTime(
    const VrptwRouteSegment &segment) {
  if(segment.segment_length == 0)
    return 0;
  auto start_time = routes_[segment.route_idx].customers[segment.segment_start_idx].travel_time_up_to;
  auto end_time = routes_[segment.route_idx].customers[segment.segment_start_idx + segment.segment_length - 1].travel_time_up_to;
  return end_time - start_time;
}
uint VrptwIndividualStructured::getSegmentDemand(
    const VrptwRouteSegment &segment) {
  if(segment.segment_length == 0)
    return 0;
  auto demand_before = segment.segment_start_idx == 0 ? 0 : routes_[segment.route_idx].customers[segment.segment_start_idx - 1].demand_up_to;
  auto demand_after = routes_[segment.route_idx].customers[segment.segment_start_idx + segment.segment_length - 1].demand_up_to;
  return demand_after - demand_before;
}

uint VrptwIndividualStructured::getExchangeTravelTime(
    const VrptwRouteSegment &remove_segment,
    const VrptwRouteSegment &insert_segment) {
  const auto &cust_to = routes_[remove_segment.route_idx].customers;
  const auto &cust_from = routes_[insert_segment.route_idx].customers;
  uint prev_node = 0;
  uint prev_time = 0;
  if(remove_segment.segment_start_idx > 0){
    prev_node = cust_to[remove_segment.segment_start_idx - 1].idx;
    prev_time = cust_to[remove_segment.segment_start_idx - 1].travel_time_up_to;
  }
  uint next_node = 0;
  uint after_time = 0;
  const uint after_idx = remove_segment.segment_start_idx + remove_segment.segment_length;
  if(after_idx < cust_to.size()){
    next_node = cust_to[after_idx].idx;
    after_time = routes_[remove_segment.route_idx].travel_time - cust_to[after_idx].travel_time_up_to;
  }

  if(insert_segment.segment_length == 0){
    return prev_time + instance_->getDistance(prev_node, next_node) + after_time;
  }
  const uint first_node = cust_from[insert_segment.segment_start_idx].idx;
  const uint last_node = cust_from[insert_segment.segment_start_idx +
                                   insert_segment.segment_length - 1].idx;

  return prev_time + instance_->getDistance(prev_node, first_node) + getSegmentTravelTime(insert_segment) + instance_->getDistance(last_node, next_node) + after_time;
}

uint VrptwIndividualStructured::getArrivalTime(
    const VrptwIndividualRoute &route, const uint &customer_idx) {
  assert(customer_idx < route.customers.size());
  if(customer_idx == 0)
    return instance_->getDistance(0, route.customers[customer_idx].idx);

  const auto &nodes = instance_->getNodes();
  const auto &prev_customer = route.customers[customer_idx - 1];
  return prev_customer.time_up_to + nodes[prev_customer.idx].service_time + instance_->getDistance(prev_customer.idx, route.customers[customer_idx].idx);
}

uint VrptwIndividualStructured::getEndTime(const VrptwIndividualRoute &route,
                                           const int &customer_idx) {
  if(customer_idx == -1)
    return 0;
  assert(customer_idx < (int)route.customers.size());
  return route.customers[customer_idx].time_up_to + instance_->getNodes()[route.customers[customer_idx].idx].service_time;
}

bool VrptwIndividualStructured::testRelocateMove(
    const VrptwRouteSegment &segment_moved,
    const VrptwRouteSegment &target_pos) {
  assert(segment_moved.route_idx != target_pos.route_idx);
  assert(segment_moved.route_idx < routes_.size() && target_pos.route_idx < routes_.size());
  const auto &route_from = routes_[segment_moved.route_idx];
  const auto &route_to = routes_[target_pos.route_idx];
  assert(segment_moved.segment_start_idx < route_from.customers.size() && segment_moved.segment_start_idx + segment_moved.segment_length <= route_from.customers.size());
  assert(target_pos.segment_start_idx <= route_to.customers.size() && target_pos.segment_length == 0);
  if(
      route_from.time_violation > 0 ||
      route_to.time_violation > 0 ||
      route_from.demand > (uint)instance_->getVehicleCapacity() ||
      route_to.demand > (uint)instance_->getVehicleCapacity()
  )
    return testExchangeMoveViolation(target_pos, segment_moved);
  else
    return testExchangeMoveNoViolation(target_pos, segment_moved);
}

bool VrptwIndividualStructured::testCrossMove(
    const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2) {
  auto segment1_ = segment1;
  auto segment2_ = segment2;
  segment1_.segment_length = routes_[segment1.route_idx].customers.size() - segment1.segment_start_idx;
  segment2_.segment_length = routes_[segment2.route_idx].customers.size() - segment2.segment_start_idx;
  return testExchangeMove(segment1_, segment2_);
}

bool VrptwIndividualStructured::test2optMoveNoViolations(
    const VrptwRouteSegment &segment) {
  // find out if the traveled distance after exchange will be lower than before
  const uint end_idx = segment.segment_start_idx + segment.segment_length - 1;
  const auto &customers = routes_[segment.route_idx].customers;
  const uint pre_node = segment.segment_start_idx == 0 ? 0 : customers[segment.segment_start_idx - 1].idx;
  const uint post_node = end_idx + 1 >= customers.size() ? 0 : customers[end_idx + 1].idx;
  const uint first_node = customers[segment.segment_start_idx].idx;
  const uint last_node = customers[end_idx].idx;

  const uint old_length = instance_->getDistance(pre_node, first_node) + instance_->getDistance(last_node, post_node);
  const uint new_length = instance_->getDistance(pre_node, last_node) + instance_->getDistance(first_node, post_node);

  if(new_length >= old_length)
    return false;

  const auto &nodes = instance_->getNodes();
  uint time = getEndTime(routes_[segment.route_idx], (int)segment.segment_start_idx - 1);
  uint prev_node = pre_node;
  for(int c = (int)end_idx; c >= (int)segment.segment_start_idx; c--){
    const auto &customer = customers[c];
    time += instance_->getDistance(prev_node, customer.idx);
    if(time > (uint)nodes[customer.idx].due_date)
      return false;
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    time += nodes[customer.idx].service_time;
    prev_node = customer.idx;
  }


  for(uint c = end_idx + 1; c < customers.size(); c++){
    const auto &customer = customers[c];
    time += instance_->getDistance(prev_node, customers[c].idx);
    if(time < customer.time_up_to)
      return true;
    if(time > (uint)nodes[customer.idx].due_date)
      return false;
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    time += nodes[customer.idx].service_time;
    prev_node = customer.idx;
  }
  return true;
}

bool VrptwIndividualStructured::test2optMoveViolation(
    const VrptwRouteSegment &segment) {
  const auto &route = routes_[segment.route_idx];

  //Find time violation of the exchanged segment
  const auto &nodes = instance_->getNodes();
  uint time = getEndTime(routes_[segment.route_idx], (int)segment.segment_start_idx - 1);
  uint prev_node = segment.segment_start_idx == 0 ? 0 : route.customers[segment.segment_start_idx - 1].idx;
  uint time_violation = 0;
  uint prev_time_violation = 0;
  for(int c = (int)segment.segment_start_idx + (int)segment.segment_length - 1; c >= (int)segment.segment_start_idx; c--){
    const auto &customer = route.customers[c];
    time += instance_->getDistance(prev_node, customer.idx);
    if(time > (uint)nodes[customer.idx].due_date)
      time_violation += time - nodes[customer.idx].due_date;
    if(customer.time_up_to > (uint)nodes[customer.idx].due_date)
      prev_time_violation += customer.time_up_to - nodes[customer.idx].due_date;
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    time += nodes[customer.idx].service_time;
    prev_node += customer.idx;
  }

  for(uint c = segment.segment_start_idx + segment.segment_length; c < route.customers.size(); c++){
    const auto &customer = route.customers[c];
    time += instance_->getDistance(prev_node, customer.idx);
    if(time > (uint)nodes[customer.idx].due_date)
      time_violation += time - nodes[customer.idx].due_date;
    if(customer.time_up_to > (uint)nodes[customer.idx].due_date)
      prev_time_violation += customer.time_up_to - nodes[customer.idx].due_date;
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    if(time == customer.time_up_to)
      break;
    time += nodes[customer.idx].service_time;
    prev_node += customer.idx;
  }

  return time_violation < prev_time_violation;
}

bool VrptwIndividualStructured::testExchangeMoveNoViolation(
    const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2) {
  assert(segment1.route_idx != segment2.route_idx);
  assert(segment1.route_idx < routes_.size() && segment2.route_idx < routes_.size());
  const auto &route1 = routes_[segment1.route_idx];
  const auto &route2 = routes_[segment2.route_idx];
  assert(segment1.segment_start_idx <= route1.customers.size() && segment1.segment_start_idx + segment1.segment_length <= route1.customers.size());
  assert(segment2.segment_start_idx <= route2.customers.size() && segment2.segment_start_idx + segment2.segment_length <= route2.customers.size());
  const auto &capacity = (uint)instance_->getVehicleCapacity();
  const uint demand1 = getSegmentDemand(segment1);
  const uint demand2 = getSegmentDemand(segment2);
  const uint demand1_new = route1.demand - demand1 + demand2;
  const uint demand2_new = route2.demand - demand2 + demand1;

  if(demand1_new > capacity || demand2_new > capacity) // violates demand constraints
    return false;

  const uint travel_time_old = route1.travel_time + route2.travel_time;
  const uint travel_time_new = getExchangeTravelTime(segment1, segment2) + getExchangeTravelTime(segment2, segment1);

  int new_empty_count = 0;
  new_empty_count += route1.customers.empty() ? -1 : 0;
  new_empty_count += route2.customers.empty() ? -1 : 0;
  new_empty_count += route1.customers.size() + segment2.segment_length - segment1.segment_length == 0 ? 1 : 0;
  new_empty_count += route2.customers.size() + segment1.segment_length - segment2.segment_length == 0 ? 1 : 0;
  if(new_empty_count < 0 || (new_empty_count == 0 && travel_time_new >= travel_time_old)) // does not improve objective
    return false;

  if(exchangeViolatesTimeConstraints(segment1, segment2) || exchangeViolatesTimeConstraints(segment2, segment1)){ // violates time constraints
    return false;
  }

  return true;
}

bool VrptwIndividualStructured::testExchangeMoveViolation(
    const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2) {

  assert(segment1.route_idx != segment2.route_idx);
  assert(segment1.route_idx < routes_.size() && segment2.route_idx < routes_.size());
  const auto &route1 = routes_[segment1.route_idx];
  const auto &route2 = routes_[segment2.route_idx];
  assert(segment1.segment_start_idx <= route1.customers.size() && segment1.segment_start_idx + segment1.segment_length <= route1.customers.size());
  assert(segment2.segment_start_idx <= route2.customers.size() && segment2.segment_start_idx + segment2.segment_length <= route2.customers.size());
  const auto &capacity = instance_->getVehicleCapacity();
  const uint demand1 = getSegmentDemand(segment1);
  const uint demand2 = getSegmentDemand(segment2);
  const uint demand1_new = route1.demand - demand1 + demand2;
  const uint demand2_new = route2.demand - demand2 + demand1;
  const int demand_violation_old = std::max(0, (int)demand1 - capacity) + std::max(0, (int)demand2 - capacity);
  const int demand_violation_new = std::max(0, (int)demand1_new - capacity) + std::max(0, (int)demand2_new - capacity);

  if(demand_violation_old < demand_violation_new)
    return false;

  int time_violation_change = exchangeTimeViolationChange(segment1, segment2);
  time_violation_change += exchangeTimeViolationChange(segment2, segment1);

  return time_violation_change < 0;
}

bool VrptwIndividualStructured::exchangeViolatesTimeConstraints(
    const VrptwRouteSegment &remove_segment,
    const VrptwRouteSegment &insert_segment) {
  const auto &to_route = routes_[remove_segment.route_idx];
  const auto &from_route = routes_[insert_segment.route_idx];
  const auto &nodes = instance_->getNodes();
  uint time = getEndTime(to_route, (int)remove_segment.segment_start_idx - 1);
  uint prev_node = remove_segment.segment_start_idx > 0 ? to_route.customers[remove_segment.segment_start_idx - 1].idx : 0;
  for(uint c = insert_segment.segment_start_idx; c < insert_segment.segment_start_idx + insert_segment.segment_length; c++){
    const auto &customer = from_route.customers[c];
    time += instance_->getDistance(prev_node, customer.idx);
    if(time > (uint)nodes[customer.idx].due_date)
      return true;
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    time += (uint)nodes[customer.idx].service_time;
    prev_node = customer.idx;
  }

  for(uint c = remove_segment.segment_start_idx + remove_segment.segment_length; c < to_route.customers.size(); c++){
    const auto &customer = to_route.customers[c];
    time += instance_->getDistance(prev_node, customer.idx);
    /*if(time <= customer.time_up_to)
      break;*/
    if(time > (uint)nodes[customer.idx].due_date)
      return true;
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    time += (uint)nodes[customer.idx].service_time;
    prev_node = customer.idx;
  }
  return false;
}
int VrptwIndividualStructured::exchangeTimeViolationChange(
    const VrptwRouteSegment &remove_segment,
    const VrptwRouteSegment &insert_segment) {
  const auto &to_route = routes_[remove_segment.route_idx];
  const auto &from_route = routes_[insert_segment.route_idx];
  const auto &nodes = instance_->getNodes();
  int time_violation_old = 0;
  int time_violation_new = 0;
  uint time = getEndTime(to_route, (int)remove_segment.segment_start_idx - 1);
  uint prev_node = remove_segment.segment_start_idx > 0 ? to_route.customers[remove_segment.segment_start_idx - 1].idx : 0;
  for(uint c = insert_segment.segment_start_idx; c < insert_segment.segment_start_idx + insert_segment.segment_length; c++){
    const auto &customer = from_route.customers[c];
    time += instance_->getDistance(prev_node, customer.idx);
    if(time > (uint)nodes[customer.idx].due_date)
      time_violation_new += (int)time - nodes[customer.idx].due_date;
    if(customer.time_up_to > (uint)nodes[customer.idx].due_date)
      time_violation_old += (int)customer.time_up_to - nodes[customer.idx].due_date;
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    time += (uint)nodes[customer.idx].service_time;
    prev_node = customer.idx;
  }

  for(uint c = remove_segment.segment_start_idx + remove_segment.segment_length; c < to_route.customers.size(); c++){
    const auto &customer = to_route.customers[c];
    time += instance_->getDistance(prev_node, customer.idx);
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    if(time == customer.time_up_to)
      break;
    if(time > (uint)nodes[customer.idx].due_date)
      time_violation_new += (int)time - nodes[customer.idx].due_date;
    if(customer.time_up_to > (uint)nodes[customer.idx].due_date)
      time_violation_old += (int)customer.time_up_to - nodes[customer.idx].due_date;
    time += (uint)nodes[customer.idx].service_time;
    prev_node = customer.idx;
  }

  return time_violation_new - time_violation_old;
}
bool VrptwIndividualStructured::assertEvaluation(
    const VrptwIndividualRoute &route) {
  const auto &nodes = instance_->getNodes();

  uint time = 0;
  uint travel_time = 0;
  uint demand = 0;
  uint time_violation = 0;
  uint prev_node = 0;
  for(uint c = 0; c < route.customers.size(); c++){
    const auto &customer = route.customers[c];
    time += instance_->getDistance(prev_node, customer.idx);
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    travel_time += instance_->getDistance(prev_node, customer.idx);
    demand += nodes[customer.idx].demand;
    assert(time == customer.time_up_to);
    assert(travel_time == customer.travel_time_up_to);
    assert(demand == customer.demand_up_to);
    if(time > (uint)nodes[customer.idx].due_date)
      time_violation += time - (uint)nodes[customer.idx].due_date;
    time += nodes[customer.idx].service_time;

    prev_node = customer.idx;
  }

  travel_time += instance_->getDistance(prev_node, 0);
  time += instance_->getDistance(prev_node, 0);
  assert(travel_time == route.travel_time);
  assert(demand == route.demand);
  assert(time == route.time);
  assert(time_violation == route.time_violation);
  return true;
}
std::vector<uint> VrptwIndividualStructured::flatten() {
  std::vector<uint> result;
  result.reserve(instance_->getNodesCount() + instance_->getVehicleCount() - 1);
  std::vector<uint> lowest_customer_idx(routes_.size(), instance_->getNodesCount());
  std::vector<uint> routes_sorted(routes_.size(), 0);
  for(uint i = 0; i < routes_.size(); i++){
    routes_sorted[i] = i;
    for(uint j = 0; j < routes_[i].customers.size(); j++){
      if(routes_[i].customers[j].idx < lowest_customer_idx[i]){
        lowest_customer_idx[i] = routes_[i].customers[j].idx;
      }
    }
  }
  std::sort(routes_sorted.begin(), routes_sorted.end(), [&](uint i, uint j){
    return lowest_customer_idx[i] < lowest_customer_idx[j];
  });

  for(uint i = 0; i < routes_.size(); i++){
    const auto &route = routes_[routes_sorted[i]];
    for(uint j = 0; j < route.customers.size(); j++){
      result.push_back(route.customers[j].idx);
    }
    result.push_back(instance_->getNodesCount() + i);
  }
  return result;
}

FitnessDiff
VrptwIndividualStructured::get2optMoveCost(const VrptwRouteSegment &segment) {
  if(segment.segment_length < 2)
    return {0, 0, 0};
  const uint end_idx = segment.segment_start_idx + segment.segment_length - 1;
  assert(segment.route_idx < routes_.size());
  const auto &customers = routes_[segment.route_idx].customers;
  assert(segment.segment_start_idx < customers.size() && end_idx < customers.size());
  uint prev_node = segment.segment_start_idx == 0 ? 0 : customers[segment.segment_start_idx - 1].idx;
  const uint next_node = end_idx + 1 >= customers.size() ? 0 : customers[end_idx + 1].idx;
  const uint first_node = customers[segment.segment_start_idx].idx;
  const uint last_node = customers[end_idx].idx;

  const uint old_length = instance_->getDistance(prev_node, first_node) + instance_->getDistance(last_node, next_node);
  const uint new_length = instance_->getDistance(prev_node, last_node) + instance_->getDistance(first_node, next_node);

  const auto &nodes = instance_->getNodes();
  uint time = getEndTime(routes_[segment.route_idx], (int)segment.segment_start_idx - 1);;
  uint time_violation = 0;
  uint old_time_violation = 0;
  for(int c = segment.segment_start_idx + segment.segment_length - 1; c >= (int)segment.segment_start_idx; c--){
    const auto &customer = routes_[segment.route_idx].customers[c];
    time += instance_->getDistance(prev_node, customer.idx);
    if(time > (uint)nodes[customer.idx].due_date){
      time_violation += time - nodes[customer.idx].due_date;
    }
    if(customer.time_up_to > (uint)nodes[customer.idx].due_date){
      old_time_violation += customer.time_up_to - nodes[customer.idx].due_date;
    }
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    time += nodes[customer.idx].service_time;
    prev_node = customer.idx;
  }

  for(uint c = segment.segment_start_idx + segment.segment_length; c < customers.size(); c++){
    const auto &customer = routes_[segment.route_idx].customers[c];
    time += instance_->getDistance(prev_node, customer.idx);
    if(time > (uint)nodes[customer.idx].due_date){
      time_violation += time - nodes[customer.idx].due_date;
    }
    if(customer.time_up_to > (uint)nodes[customer.idx].due_date){
      old_time_violation += customer.time_up_to - nodes[customer.idx].due_date;
    }
    if(time == customer.time_up_to)
      break;
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    time += nodes[customer.idx].service_time;
    prev_node = customer.idx;
  }

  return {(int)new_length - (int)old_length, (int)time_violation - (int)old_time_violation, 0};
}

FitnessDiff VrptwIndividualStructured::getExchangeMoveCost(
    const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2) {
  assert(segment1.route_idx != segment2.route_idx);
  assert(segment1.route_idx < routes_.size() && segment2.route_idx < routes_.size());
  const auto &route1 = routes_[segment1.route_idx];
  const auto &route2 = routes_[segment2.route_idx];
  assert(segment1.segment_start_idx <= route1.customers.size() && segment1.segment_start_idx + segment1.segment_length <= route1.customers.size());
  assert(segment2.segment_start_idx <= route2.customers.size() && segment2.segment_start_idx + segment2.segment_length <= route2.customers.size());
  const auto &capacity = instance_->getVehicleCapacity();
  const uint demand1 = getSegmentDemand(segment1);
  const uint demand2 = getSegmentDemand(segment2);
  const uint demand1_new = route1.demand - demand1 + demand2;
  const uint demand2_new = route2.demand - demand2 + demand1;
  const int demand_violation_old = std::max(0, (int)demand1 - capacity) + std::max(0, (int)demand2 - capacity);
  const int demand_violation_new = std::max(0, (int)demand1_new - capacity) + std::max(0, (int)demand2_new - capacity);

  int time_violation_change = exchangeTimeViolationChange(segment1, segment2);
  time_violation_change += exchangeTimeViolationChange(segment2, segment1);

  const uint travel_time_old = route1.travel_time + route2.travel_time;
  const uint travel_time_new = getExchangeTravelTime(segment1, segment2) + getExchangeTravelTime(segment2, segment1);

  int new_empty_count = 0;
  new_empty_count += route1.customers.empty() ? -1 : 0;
  new_empty_count += route2.customers.empty() ? -1 : 0;
  new_empty_count += route1.customers.size() + segment2.segment_length - segment1.segment_length == 0 ? 1 : 0;
  new_empty_count += route2.customers.size() + segment1.segment_length - segment2.segment_length == 0 ? 1 : 0;


  return {
      (int)travel_time_new - (int)travel_time_old,
      (int)time_violation_change + demand_violation_new - demand_violation_old,
      -new_empty_count
  };
}

FitnessDiff VrptwIndividualStructured::getRelocateMoveCost(
    const VrptwRouteSegment &segment_moved,
    const VrptwRouteSegment &target_pos) {
  assert(segment_moved.route_idx != target_pos.route_idx);
  assert(segment_moved.route_idx < routes_.size() && target_pos.route_idx < routes_.size());
  const auto &route_from = routes_[segment_moved.route_idx];
  const auto &route_to = routes_[target_pos.route_idx];
  assert(segment_moved.segment_start_idx < route_from.customers.size() && segment_moved.segment_start_idx + segment_moved.segment_length <= route_from.customers.size());
  assert(target_pos.segment_start_idx <= route_to.customers.size() && target_pos.segment_length == 0);

  return getExchangeMoveCost(target_pos, segment_moved);
}

FitnessDiff
VrptwIndividualStructured::getCrossMoveCost(const VrptwRouteSegment &segment1,
                                            const VrptwRouteSegment &segment2) {
  auto segment1_ = segment1;
  auto segment2_ = segment2;
  segment1_.segment_length = routes_[segment1.route_idx].customers.size() - segment1.segment_start_idx;
  segment2_.segment_length = routes_[segment2.route_idx].customers.size() - segment2.segment_start_idx;
  return getExchangeMoveCost(segment1_, segment2_);
}

FitnessDiff VrptwIndividualStructured::getFitnessDiff(
    const VrptwIndividualStructured &other) {
  return {
      (int)total_travel_time_ - (int)other.total_travel_time_,
      (int)(violations_[0] + violations_[1]) - (int)(other.violations_[0] + other.violations_[1]),
      (int)vehicles_used_ - (int)other.vehicles_used_
  };
}

std::pair<uint, uint> VrptwIndividualStructured::andvanceSegmentTimeViolation(
    const VrptwRouteSegment &segment, const uint &prev_customer,
    const std::pair<uint, uint> &prev_time_violation,
    bool terminate_on_match) {
  const auto &nodes = instance_->getNodes();
  uint time = prev_time_violation.first;
  uint prev_node = prev_customer;
  uint time_violation = prev_time_violation.second;
  for(uint c = segment.segment_start_idx; c < segment.segment_start_idx + segment.segment_length; c++){
    const auto &customer = routes_[segment.route_idx].customers[c];
    time += instance_->getDistance(prev_node, customer.idx);
    if(time > (uint)nodes[customer.idx].due_date){
      time_violation += time - nodes[customer.idx].due_date;
    }
    time = std::max(time, (uint)nodes[customer.idx].ready_time);
    if(terminate_on_match && time == customer.time_up_to)
      return {time, time_violation};
    time += nodes[customer.idx].service_time;
    prev_node = customer.idx;
  }
  return {time, time_violation};
}
