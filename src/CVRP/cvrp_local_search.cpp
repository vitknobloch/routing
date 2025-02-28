//
// Created by knoblvit on 24.2.25.
//
#include "CVRP/cvrp_local_search.h"
#include <cassert>
#include <iostream>
#include <random>

CvrpLocalSearch::CvrpLocalSearch(
    const std::shared_ptr<RoutingInstance> &instance,
    const std::shared_ptr<Mutation> &mutation) {
  instance_ = instance;
  best_solution_ = nullptr;
  terminate_ = false;
  matrix_ = instance->getMatrix().get();
  local_search_ = nullptr;
  mutation_ = mutation;
}

std::shared_ptr<Solution> CvrpLocalSearch::convertSolution(
    const std::shared_ptr<CvrpIndividual> &individual) {
  const auto &data = individual->data();
  uint zero_idx; // Find the first node index
  for(zero_idx = 0; zero_idx < data.size(); zero_idx++){
    if(data[zero_idx] == 0)
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

  uint i = (zero_idx + 1) % data.size();
  uint prev_node = 0;
  uint time = 0;
  uint travel_time = 0;
  uint demand = 0;
  while(i != zero_idx){
    // update running sums
    const uint cur_node = data[i] >= (uint)instance_->getNodesCount() ? 0 : data[i];
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
    i = (i + 1) % data.size();
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

  assert(solution->routes.size() == instance_->getVehicleCount());

  solution->end_time_sum += time;
  solution->travel_time_sum += travel_time;
  solution->objective = solution->travel_time_sum;

  solution->feasible = individual->getTotalConstraintViolation() == 0;

  return solution;
}

std::shared_ptr<CvrpIndividual>
CvrpLocalSearch::convertSolution(const std::shared_ptr<Solution> &solution) {
  auto individual = std::make_shared<CvrpIndividual>(instance_.get());
  individual->initialize();
  auto &data = individual->data();

  uint i = 0;
  uint depot_alias = instance_->getNodesCount() - 1;
  for(const auto &route: solution->routes){
    data[i] = depot_alias;
    i++;
    depot_alias++;
    for(const auto &node: route.route_nodes){
      if(node.idx == 0) //skip depots
        continue;
      data[i] = node.idx;
      i++;
    }
  }
  data[0] = 0;

  individual->setFitness(solution->objective);
  individual->calculateConstraints();
  return individual;
}

void CvrpLocalSearch::sendSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution == nullptr || portfolio_ == nullptr)
    return;
  portfolio_->acceptSolution(solution);
}

void CvrpLocalSearch::initialize(HeuristicPortfolio *portfolio) {
  portfolio_ = portfolio;
  terminate_ = false;
  std::shared_ptr<Callbacks> callbacks = std::make_shared<Callbacks>();
  callbacks->setTerminationCondition([this]() -> bool {
    return terminate_;
  });
  callbacks->addNewBestSolutionCallback([this](const std::shared_ptr<Individual> &individual) {
    auto individual_ = std::static_pointer_cast<CvrpIndividual>(individual);
    auto solution = convertSolution(individual_);
    //std::cerr << solution->objective << " " << individual->getTotalConstraintViolation() << std::endl;
    sendSolution(solution);
  });
  local_search_ = std::make_shared<LocalSearch>(callbacks, mutation_);
}

void CvrpLocalSearch::terminate() {
  terminate_.store(true);
}

void CvrpLocalSearch::acceptSolution(std::shared_ptr<Solution> solution) {
  if(checkBetterSolution(solution)){
    auto individual= convertSolution(solution);
    local_search_->acceptOutsideSolution(individual);
  }
}

bool CvrpLocalSearch::checkBetterSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution->objective < 0) { // integer overflow
    std::cerr << "Integer overflow encountered in CVRP local search solution value" << std::endl;
    return false;
  }
  std::lock_guard<std::recursive_mutex> lock(solution_mutex_);
  if(best_solution_ == nullptr || solution->objective < best_solution_->objective){
    best_solution_ = solution;
    return true;
  }
  return false;
}

void CvrpLocalSearch::run(){
  std::random_device rand;
  std::mt19937 gen(rand());
  auto initialSolution = std::make_shared<CvrpIndividual>(instance_.get());
  initialSolution->initialize();
  std::shuffle(initialSolution->data().begin(), initialSolution->data().end(), gen);
  initialSolution->resetEvaluated();
  initialSolution->evaluate();
  auto solution = convertSolution(initialSolution);
  portfolio_->acceptSolution(solution);
  local_search_->run(initialSolution);
}