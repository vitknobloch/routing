#include "CVRP/cvrp_individual.h"
#include <cassert>
#include <limits>

CvrpIndividual::CvrpIndividual(const RoutingInstance *const instance) :
 instance_(instance), matrix_(instance->getMatrix().get()), data_(), is_evaluated_(false),
  fitness_(std::numeric_limits<double>::max()), capacity_constraint_violation_(0){}

CvrpIndividual::CvrpIndividual(const CvrpIndividual &other) :
instance_(other.instance_), matrix_(other.matrix_), data_(other.data_), is_evaluated_(other.is_evaluated_),
  fitness_(other.fitness_), capacity_constraint_violation_(other.capacity_constraint_violation_){}

void CvrpIndividual::initialize() {
  data_.clear();
  data_.reserve(instance_->getNodesCount() + instance_->getVehicleCount() - 1);
  for(int i = 0; i < instance_->getNodesCount() + instance_->getVehicleCount() - 1; i++){
    data_.push_back(i);
  }
  resetEvaluated();
}

std::vector<uint> &CvrpIndividual::data() { return data_; }

void CvrpIndividual::resetEvaluated() {
  is_evaluated_ = false;
}

void CvrpIndividual::setFitness(double fitness) {
  fitness_ = fitness;
  is_evaluated_ = true;
}

bool CvrpIndividual::betterThan(const std::shared_ptr<Individual> &other) {
  if(!other)
    return true;
  auto individual_ = std::static_pointer_cast<CvrpIndividual>(other);
  if(capacity_constraint_violation_ < other->getTotalConstraintViolation())
    return true;
  else if(capacity_constraint_violation_ == other->getTotalConstraintViolation() &&
           fitness_ < other->getFitness())
    return true;
  return false;
}

void CvrpIndividual::calculateFitness() {
  fitness_ = 0;
  for(uint i = 0; i < data_.size() - 1; i++){
    fitness_ += instance_->getDistance(data_[i], data_[i+1]);
  }
  fitness_ += instance_->getDistance(data_[data_.size() - 1], data_[0]);
}

std::shared_ptr<Individual> CvrpIndividual::deepcopy() {
  return std::make_shared<CvrpIndividual>(*this);
}
double CvrpIndividual::getFitness() {
  if(!is_evaluated_)
    evaluate();
  return fitness_;
}

bool CvrpIndividual::isEvaluated() {
  return is_evaluated_;
}

void CvrpIndividual::calculateConstraints() {
  const auto &nodes = instance_->getNodes();
  uint first_vehicle_start = data_.size();
  uint i = 0;
  uint demand_running_sum = 0;
  uint demand_violation = 0;
  while(i != first_vehicle_start){
    const uint node = data_[i] >= (uint)instance_->getNodesCount() ? 0 : data_[i];
    if(node == 0){
      if(first_vehicle_start == data_.size()){
        first_vehicle_start = i;
        demand_running_sum = 0;
      }
      demand_violation += std::max((int)demand_running_sum - instance_->getVehicleCapacity(), 0);
      demand_running_sum = 0;
    }
    demand_running_sum += nodes[node].demand;
    i = (i + 1) % data_.size();
  }
  demand_violation += std::max((int)demand_running_sum - instance_->getVehicleCapacity(), 0);
  capacity_constraint_violation_ = demand_violation;
}
const std::vector<double> &CvrpIndividual::getConstraintViolations() {
  return {capacity_constraint_violation_};
}

double CvrpIndividual::getTotalConstraintViolation() {
  return capacity_constraint_violation_;
}

void CvrpIndividual::evaluate() {
  if(!is_evaluated_){
    calculateFitness();
    calculateConstraints();
  }
  is_evaluated_ = true;
}

bool CvrpIndividual::assertData() {
  std::vector<bool> included(data_.size(), false);
  for(const auto &n: data_){
    included[n] = true;
  }
  for(const auto &i : included)
    if(!included[i])
      return false;
  return true;
}

CvrpIndividual::CvrpIndividual(const RoutingInstance *const instance,
                               const std::shared_ptr<Solution> &solution) : CvrpIndividual(instance) {

  data_.reserve(instance_->getNodesCount() + instance_->getVehicleCount() - 1);
  for(int i = 0; i < instance_->getNodesCount() + instance_->getVehicleCount() - 1; i++){
    data_.push_back(i);
  }

  uint i = 0;
  uint capacity_violation = 0;
  uint depot_alias = instance_->getNodesCount() - 1;
  for(const auto &route: solution->routes){
    data_[i] = depot_alias;
    i++;
    depot_alias++;
    for(const auto &node: route.route_nodes){
      if(node.idx == 0) //skip depots
        continue;
      data_[i] = node.idx;
      i++;
    }
    capacity_violation += std::max(0, route.demand - instance_->getVehicleCapacity());
  }
  data_[0] = 0;

  fitness_ = solution->objective;
  capacity_constraint_violation_ = capacity_violation;
}

std::shared_ptr<Solution> CvrpIndividual::convertSolution() {
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
  while(i != zero_idx){
    // update running sums
    const uint cur_node = data_[i] >= (uint)instance_->getNodesCount() ? 0 : data_[i];
    const uint transition = instance_->getDistance(prev_node, cur_node);
    time += transition;
    travel_time += transition;
    demand += nodes[cur_node].demand;

    // add new node to solution
    SolutionNode node;
    node.idx = cur_node;
    node.start_time = time;
    time++;
    node.end_time = time;
    solution->routes.back().route_nodes.push_back(node);

    if(cur_node == 0){ // end of route
      time--;
      // finish route
      solution->routes.back().end_time = time;
      solution->routes.back().travel_time = travel_time;
      solution->routes.back().demand = demand;
      solution->routes.back().route_nodes.back().end_time = time;

      // add costs to solution costs
      solution->end_time_sum += time;
      solution->travel_time_sum += travel_time;

      // initialize next route
      solution->routes.emplace_back();
      solution->routes.back().route_nodes.push_back(first_node);

      //reset running sums
      time = 0;
      travel_time = 0;
      demand = 0;
    }

    prev_node = cur_node;
    i = (i + 1) % data_.size();
  }

  //Finish last route
  // update running sums
  const uint transition_last = instance_->getDistance(prev_node, 0);
  time += transition_last;
  travel_time += transition_last;
  // add last node to solution
  SolutionNode node;
  node.idx = 0;
  node.start_time = time;
  node.end_time = time;
  solution->routes.back().route_nodes.push_back(node);
  // finish route
  solution->routes.back().travel_time = travel_time;
  solution->routes.back().demand = demand;
  solution->routes.back().end_time = time;

  assert(solution->routes.size() == (uint)instance_->getVehicleCount());

  solution->end_time_sum += time;
  solution->travel_time_sum += travel_time;
  solution->objective = solution->travel_time_sum;
  solution->used_vehicles = instance_->getVehicleCount();

  solution->feasible = getTotalConstraintViolation() == 0;

  return solution;
}

void CvrpIndividual::smartInitialize() {
  initialize();
}
