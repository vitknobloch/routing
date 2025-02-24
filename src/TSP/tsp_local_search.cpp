//
// Created by knoblvit on 23.2.25.
//
#include "TSP/tsp_local_search.h"
#include <iostream>
#include <random>

TspLocalSearch::TspLocalSearch(
    const std::shared_ptr<RoutingInstance> &instance,
    const std::shared_ptr<Mutation> &mutation) {
  instance_ = instance;
  best_solution_ = nullptr;
  terminate_ = false;
  matrix_ = instance->getMatrix().get();
  local_search_ = nullptr;
  mutation_ = mutation;
}

std::shared_ptr<Solution> TspLocalSearch::convertSolution(
    const std::shared_ptr<TspIndividual> &individual) {
  const auto &data = individual->data();
  uint zero_idx; // Find the first node index
  for(zero_idx = 0; zero_idx < data.size(); zero_idx++){
    if(data[zero_idx] == 0)
      break;
  }
  // Fill out nodes
  auto nodes = std::list<SolutionNode>();
  int start_time = 0;
  for(uint i = 0; i < data.size(); i++){
    auto node = SolutionNode();
    node.idx = (int)data[(zero_idx + i) % data.size()];
    node.start_time = start_time;
    node.end_time = start_time + 1;
    const uint next_node = data[(zero_idx + i + 1) % data.size()];
    start_time += 1 + matrix_[node.idx * data.size() + next_node];
    nodes.push_back(node);
  }
  auto last_node = SolutionNode();
  last_node.idx = 0;
  last_node.start_time = start_time;
  last_node.end_time = start_time + 1;
  nodes.push_back(last_node);

  // Fill out objectives
  auto solution = std::make_shared<Solution>();
  const auto objective = (int)individual->getFitness();
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


  return solution;
}

std::shared_ptr<TspIndividual>
TspLocalSearch::convertSolution(const std::shared_ptr<Solution> &solution) {
  auto individual = std::make_shared<TspIndividual>(instance_.get());
  individual->initialize();
  auto &data = individual->data();
  uint i = 0;
  for(const auto &node: solution->routes.front().route_nodes){
    data[i] = node.idx;
    i++;
    if(i >= data.size())
      break;
  }
  individual->setFitness(solution->objective);
  return individual;
}

void TspLocalSearch::sendSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution == nullptr || portfolio_ == nullptr)
    return;
  portfolio_->acceptSolution(solution);
}

void TspLocalSearch::initialize(HeuristicPortfolio *portfolio) {
  portfolio_ = portfolio;
  terminate_ = false;
  std::shared_ptr<Callbacks> callbacks = std::make_shared<Callbacks>();
  callbacks->setTerminationCondition([this]() -> bool {
    return terminate_;
  });
  callbacks->addNewBestSolutionCallback([this](const std::shared_ptr<Individual> &individual) {
    auto individual_ = std::static_pointer_cast<TspIndividual>(individual);
    auto solution = convertSolution(individual_);
    sendSolution(solution);
  });
  local_search_ = std::make_shared<LocalSearch>(callbacks, mutation_);
}

void TspLocalSearch::terminate() {
  terminate_.store(true);
}

void TspLocalSearch::acceptSolution(std::shared_ptr<Solution> solution) {
  if(checkBetterSolution(solution)){
    auto individual= convertSolution(solution);
    local_search_->acceptOutsideSolution(individual);
  }
}

bool TspLocalSearch::checkBetterSolution(
    const std::shared_ptr<Solution> &solution) {
  std::lock_guard<std::recursive_mutex> lock(solution_mutex_);
  if(best_solution_ == nullptr || solution->objective < best_solution_->objective){
    best_solution_ = solution;
    return true;
  }
  return false;
}

void TspLocalSearch::run(){
  std::random_device rand;
  std::mt19937 gen(rand());
  std::shared_ptr<TspIndividual> initialSolution = std::make_shared<TspIndividual>(instance_.get());
  initialSolution->initialize();
  std::shuffle(initialSolution->data().begin(), initialSolution->data().end(), gen);
  initialSolution->evaluate();
  auto solution = convertSolution(initialSolution);
  portfolio_->acceptSolution(solution);
  local_search_->run(initialSolution);
}
