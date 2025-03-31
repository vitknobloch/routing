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
    auto solution = individual_->convertSolution();
    if(checkBetterSolution(solution))
      sendSolution(solution);
  });
  local_search_ = std::make_shared<StochasticLocalSearch>(callbacks, mutation_);
}

void TspLocalSearch::terminate() {
  terminate_.store(true);
}

void TspLocalSearch::acceptSolution(std::shared_ptr<Solution> solution) {
  if(checkBetterSolution(solution)){
    auto individual= std::make_shared<TspIndividual>(instance_.get(), solution);
    local_search_->acceptOutsideSolution(individual);
  }
}

bool TspLocalSearch::checkBetterSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution->objective < 0) { // integer overflow
    std::cerr << "Integer overflow encountered in TSP local search solution value" << std::endl;
    return false;
  }
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
  initialSolution->initializeNearestNeighbor();
  //std::shuffle(initialSolution->data().begin(), initialSolution->data().end(), gen);
  initialSolution->evaluate();
  auto solution = initialSolution->convertSolution();
  if(checkBetterSolution(solution))
    portfolio_->acceptSolution(solution);
  local_search_->run(initialSolution);
}
