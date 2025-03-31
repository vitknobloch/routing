//
// Created by knoblvit on 9.3.25.
//
#include "VRP-TW/vrptw_individual.h"

#include <cassert>
#include <limits>

VrptwIndividual::VrptwIndividual(const RoutingInstance *const instance) :
  instance_(instance), matrix_(instance->getMatrix().get()), data_(), is_evaluated_(false),
  fitness_(std::numeric_limits<double>::max()), total_constraint_violation_(0), constraint_violations_(2, 0.0){}

VrptwIndividual::VrptwIndividual(const VrptwIndividual &other) :
  instance_(other.instance_), matrix_(other.matrix_), data_(other.data_), is_evaluated_(other.is_evaluated_),
  fitness_(other.fitness_), total_constraint_violation_(other.total_constraint_violation_), constraint_violations_(other.constraint_violations_){}

VrptwIndividual::VrptwIndividual(const RoutingInstance *const instance,
                                 const std::shared_ptr<Solution> &solution) : VrptwIndividual(instance){
  data_.reserve(instance_->getNodesCount() + instance_->getVehicleCount() - 1);
  for(int i = 0; i < instance_->getNodesCount() + instance_->getVehicleCount() - 1; i++){
    data_.push_back(i);
  }

  const auto &nodes = instance_->getNodes();

  uint i = 0;
  uint capacity_violation = 0;
  uint time_window_violation = 0;
  uint depot_alias = instance_->getNodesCount() - 1;
  for(const auto &route: solution->routes){
    data_[i] = depot_alias;
    i++;
    depot_alias++;
    for(const auto &node: route.route_nodes){
      if(node.idx == 0) //skip depots
        continue;
      data_[i] = node.idx;
      time_window_violation += std::max(0, nodes[node.idx].ready_time - node.start_time);
      time_window_violation += std::max(0, node.start_time - nodes[node.idx].due_date);
      i++;
    }
    capacity_violation += std::max(0, route.demand - instance_->getVehicleCapacity());
  }
  data_[0] = 0;

  fitness_ = solution->objective;
  constraint_violations_[0] = capacity_violation;
  constraint_violations_[1] = time_window_violation;

  total_constraint_violation_ = capacity_violation + time_window_violation;
}

void VrptwIndividual::initialize() {
  data_.clear();
  data_.reserve(instance_->getNodesCount() + instance_->getVehicleCount() - 1);
  for(int i = 0; i < instance_->getNodesCount() + instance_->getVehicleCount() - 1; i++){
    data_.push_back(i);
  }
  resetEvaluated();
}

std::vector<uint> &VrptwIndividual::data() { return data_; }

void VrptwIndividual::resetEvaluated() { is_evaluated_ = false; }

bool VrptwIndividual::assertData() {
  std::vector<bool> included(data_.size(), false);
  for(const auto &n: data_){
    included[n] = true;
  }
  for(const auto &i : included)
    if(!included[i])
      return false;
  return true;
}

void VrptwIndividual::setFitness(double fitness) {
  fitness_ = fitness;
  is_evaluated_ = true;
}

bool VrptwIndividual::betterThan(const std::shared_ptr<Individual> &other) {
  if(!other)
    return true;
  auto individual_ = std::static_pointer_cast<VrptwIndividual>(other);
  if(total_constraint_violation_ < other->getTotalConstraintViolation())
    return true;
  else if(total_constraint_violation_ == other->getTotalConstraintViolation() &&
           fitness_ < other->getFitness())
    return true;
  return false;
}

void VrptwIndividual::calculateFitness() {
  const auto &nodes = instance_->getNodes();
  uint first_vehicle_start;
  for(first_vehicle_start = 0; data_[first_vehicle_start] > 0 && data_[first_vehicle_start] < (uint)instance_->getNodesCount(); first_vehicle_start++);

  uint i = (first_vehicle_start + 1) % data_.size();
  fitness_ = 0;
  uint demand_running_sum = 0;
  uint demand_violation = 0;
  uint time_window_violation = 0;
  uint time = std::max((uint)nodes[data_[i]].ready_time, instance_->getDistance(0, data_[i]));
  while(i != first_vehicle_start){
    const uint node = data_[i] >= (uint)instance_->getNodesCount() ? 0 : data_[i];
    const uint next_node = data_[(i+1) % data_.size()] >= (uint)instance_->getNodesCount() ? 0 : data_[(i+1) % data_.size()];
    if(node == 0){
      demand_violation += std::max((int)demand_running_sum - instance_->getVehicleCapacity(), 0);
      demand_running_sum = 0;
      fitness_ += time;
      time = 0;
    }
    if((int)time < nodes[node].ready_time){
      time = nodes[node].ready_time;
    }
    if((int)time > nodes[node].due_date){
      time_window_violation += time - nodes[node].due_date;
    }
    demand_running_sum += nodes[node].demand;
    time += nodes[node].service_time + instance_->getDistance(node, next_node);

    i = (i + 1) % data_.size();
  }
  demand_violation += std::max((int)demand_running_sum - instance_->getVehicleCapacity(), 0);
  constraint_violations_[0] = demand_violation;
  constraint_violations_[1] = time_window_violation;
  total_constraint_violation_ = demand_violation + time_window_violation;
}

void VrptwIndividual::evaluate() {
  if(!is_evaluated_)
    calculateFitness();
  is_evaluated_ = true;
}

std::shared_ptr<Individual> VrptwIndividual::deepcopy() {
  return std::make_shared<VrptwIndividual>(*this);
}

double VrptwIndividual::getFitness() {
  evaluate();
  return fitness_;
}

bool VrptwIndividual::isEvaluated() {
  return is_evaluated_;
}

void VrptwIndividual::calculateConstraints() {
  calculateFitness();
}

const std::vector<double> &VrptwIndividual::getConstraintViolations() {
  return constraint_violations_;
}

double VrptwIndividual::getTotalConstraintViolation() { return total_constraint_violation_; }

std::shared_ptr<Solution> VrptwIndividual::convertSolution() {
  //Find zero idx
  uint zero_idx; // Find the first node index
  for(zero_idx = 0; zero_idx < data_.size(); zero_idx++){
    if(data_[zero_idx] == 0)
      break;
  }

  //Initialize solution
  const auto &nodes = instance_->getNodes();
  auto solution = std::make_shared<Solution>();
  solution->travel_time_sum = 0;
  solution->end_time_sum = 0;
  solution->objective = 0;
  solution->feasible = true;

  //initialize first route
  solution->routes.emplace_back();
  SolutionNode first_node;
  first_node.idx = 0;
  first_node.start_time = 0;
  first_node.end_time = 0;
  solution->routes.back().route_nodes.push_back(first_node);

  uint i = (zero_idx + 1) % data_.size();
  uint prev_node = 0;
  uint time = 0;
  uint travel_time = 0;
  uint demand = 0;
  while(i != zero_idx) {
    const uint cur_node =
        data_[i] >= (uint)instance_->getNodesCount() ? 0 : data_[i];
    const uint transition = instance_->getDistance(prev_node, cur_node);
    time += transition;
    travel_time += transition;
    demand += nodes[cur_node].demand;
    time = std::max(time, (uint)nodes[cur_node].ready_time);
    if (time > nodes[cur_node].due_date)
      solution->feasible = false;

    SolutionNode node;
    node.idx = cur_node;
    node.start_time = time;
    time += nodes[cur_node].service_time;
    node.end_time = time;
    solution->routes.back().route_nodes.push_back(node);

    if (cur_node == 0) { // end of route
      // finish route
      solution->routes.back().end_time = time;
      solution->routes.back().travel_time = travel_time;
      solution->routes.back().demand = demand;
      if (demand > instance_->getVehicleCapacity())
        solution->feasible = false;

      // add solution cost to solution cost
      solution->end_time_sum += time;
      solution->travel_time_sum += travel_time;
      // initialize next route
      solution->routes.emplace_back();
      solution->routes.back().route_nodes.push_back(first_node);

      time = 0;
      travel_time = 0;
      demand = 0;
    }

    prev_node = cur_node;
    i = (i+1) % data_.size();
  }

  //Finish last route
  const uint transition_last = instance_->getDistance(prev_node, 0);
  time += transition_last;
  travel_time += transition_last;

  SolutionNode node;
  node.idx = 0;
  node.start_time = time;
  node.end_time = time;
  solution->routes.back().route_nodes.push_back(node);

  solution->routes.back().travel_time = travel_time;
  solution->routes.back().end_time = time;
  solution->routes.back().demand = demand;
  if(demand > instance_->getVehicleCapacity())
    solution->feasible = false;

  assert(solution->routes.size() == (uint)(instance_->getVehicleCount()));

  solution->end_time_sum += time;
  solution->travel_time_sum += travel_time;
  solution->objective = solution->end_time_sum;

  return solution;
}

void VrptwIndividual::smartInitialize() {
  initialize();
}
