#include "TSP/tsp_individual.h"
#include <bitset>
#include <cassert>
#include <iostream>
#include <limits>

TspIndividual::TspIndividual(const RoutingInstance *const instance) : instance_(instance), matrix_(instance->getMatrix().get()), data_(), is_evaluated_(false), fitness_(std::numeric_limits<double>::max()) {

}

TspIndividual::TspIndividual(const TspIndividual &other) : instance_(other.instance_), matrix_(other.matrix_), data_(other.data_),  is_evaluated_(other.is_evaluated_), fitness_(other.fitness_){

}

void TspIndividual::initialize() {
  data_.clear();
  data_.reserve(instance_->getNodesCount());
  for(int i = 0; i < instance_->getNodesCount(); i++){
    data_.push_back(i);
  }
}

void TspIndividual::initializeNearestNeighbor(){
  data_.clear();
  data_.reserve(instance_->getNodesCount());

  const auto N = instance_->getNodesCount();
  std::vector<bool> used(N, false);

  uint start_node = rand() % N; // Should probably use structures from <random>
  data_.push_back(start_node);
  used[start_node] = true;
  for(int i = 0; i < N-1; i++){
    int nearest_neighbor = -1;
    uint best_distance = std::numeric_limits<uint>::max();
    const uint cur_node = data_.back();
    for(int j = 0; j < N; j++){
      if(used[j])
        continue;
      const uint distance = instance_->getDistance(cur_node, j);
      if(distance < best_distance){
        nearest_neighbor = j;
        best_distance = distance;
      }
    }

    used[nearest_neighbor] = true;
    data_.push_back(nearest_neighbor);
  }

  assert(data_.size() == (uint)N);
  resetEvaluated();
}

std::vector<uint> &TspIndividual::data() {
  return data_;
}

void TspIndividual::resetEvaluated() {
  is_evaluated_ = false;
}

void TspIndividual::setFitness(double fitness) {
  fitness_ = fitness;
  is_evaluated_ = true;
}

bool TspIndividual::betterThan(const std::shared_ptr<Individual> &other) {
  return !other || this->fitness_ < ((TspIndividual*)other.get())->fitness_;
}

void TspIndividual::evaluate() {
  if(!is_evaluated_){
    calculateFitness();
    is_evaluated_ = true;
  }

}

void TspIndividual::calculateFitness() {
  fitness_ = 0;
  const auto matrix_size = instance_->getNodesCount();

  for(uint i = 0 ; i < data_.size() - 1; i++){
    fitness_ += matrix_[data_[i] * matrix_size + data_[i+1]];
  }
  fitness_ += matrix_[data_[matrix_size - 1] * matrix_size + data_[0]];
}

std::shared_ptr<Individual> TspIndividual::deepcopy() {
  return std::make_shared<TspIndividual>(*this);
}

double TspIndividual::getFitness() {
  if(!is_evaluated_)
    evaluate();
  return fitness_;
}

bool TspIndividual::isEvaluated() {
  return is_evaluated_;
}

void TspIndividual::calculateConstraints() {}
const std::vector<double> &TspIndividual::getConstraintViolations() {
  return std::vector<double>();
}

double TspIndividual::getTotalConstraintViolation() { return 0; }

std::shared_ptr<Solution> TspIndividual::convertSolution() {
  uint zero_idx; // Find the first node index
  for(zero_idx = 0; zero_idx < data_.size(); zero_idx++){
    if(data_[zero_idx] == 0)
      break;
  }
  // Fill out nodes
  auto nodes = std::list<SolutionNode>();
  int start_time = 0;
  for(uint i = 0; i < data_.size(); i++){
    auto node = SolutionNode();
    node.idx = (int)data_[(zero_idx + i) % data_.size()];
    node.start_time = start_time;
    node.end_time = start_time + 1;
    const uint next_node = data_[(zero_idx + i + 1) % data_.size()];
    start_time += 1 + matrix_[node.idx * data_.size() + next_node];
    nodes.push_back(node);
  }
  auto last_node = SolutionNode();
  last_node.idx = 0;
  last_node.start_time = start_time;
  last_node.end_time = start_time + 1;
  nodes.push_back(last_node);

  // Fill out objectives
  auto solution = std::make_shared<Solution>();
  const auto objective = fitness_;
  solution->objective = objective;
  solution->travel_time_sum = objective;
  solution->end_time_sum = last_node.end_time;
  solution->routes = std::list<SolutionRoute>();
  auto route = SolutionRoute();
  route.demand = -1;
  route.end_time = solution->end_time_sum;
  route.travel_time = objective;
  route.route_nodes = nodes;
  solution->routes.push_back(route);
  solution->feasible = true;
  solution->used_vehicles = 1;

  return solution;
}

TspIndividual::TspIndividual(const RoutingInstance *const instance,
                             const std::shared_ptr<Solution> &solution) : TspIndividual(instance) {
  data_.reserve(instance_->getNodesCount());
  uint i = 0;
  for(const auto &node: solution->routes.front().route_nodes){
    data_.push_back(node.idx);
    i++;
    if(i >= (uint)instance_->getNodesCount())
      break;
  }
  setFitness(solution->objective);
}

void TspIndividual::smartInitialize() {
  initializeNearestNeighbor();
}
