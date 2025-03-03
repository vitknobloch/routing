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
    auto solution = individual_->convertSolution();
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
    auto individual= std::make_shared<CvrpIndividual>(instance_.get(), solution);
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
  auto solution = initialSolution->convertSolution();
  portfolio_->acceptSolution(solution);
  local_search_->run(initialSolution);
}