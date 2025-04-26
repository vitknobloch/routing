#include "TSP/tsp_exhaustive_local_search.h"
#include <cassert>
#include <iostream>

TspExhaustiveLocalSearch::TspExhaustiveLocalSearch(
    const std::shared_ptr<RoutingInstance> &instance,
    const std::shared_ptr<Neighborhood> &neighborhood) {
  instance_ = instance;
  best_solution_ = nullptr;
  terminate_ = false;
  local_search_ = nullptr;
  neighborhood_ = neighborhood;

  assert(instance_ != nullptr && neighborhood_ != nullptr);
}

void TspExhaustiveLocalSearch::sendSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution == nullptr || portfolio_ == nullptr)
    return;
  portfolio_->acceptSolution(solution);
}

bool TspExhaustiveLocalSearch::checkBetterSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution->objective < 0) { // integer overflow
    std::cerr << "Integer overflow encountered in CVRP exhaustive local search solution value" << std::endl;
    return false;
  }
  std::lock_guard<std::recursive_mutex> lock(solution_mutex_);
  if(best_solution_ == nullptr || solution->objective < best_solution_->objective){
    best_solution_ = solution;
    return true;
  }
  return false;
}

void TspExhaustiveLocalSearch::initialize(HeuristicPortfolio *portfolio) {
  portfolio_ = portfolio;
  terminate_ = false;
  std::shared_ptr<Callbacks> callbacks = std::make_shared<Callbacks>();
  callbacks->setTerminationCondition([this]() -> bool {
    return terminate_;
  });
  callbacks->addNewBestSolutionCallback([this](const std::shared_ptr<Individual> &individual) {
    auto individual_ = std::static_pointer_cast<TspIndividualStructured>(individual);
    auto solution = individual_->convertSolution();
    //std::cerr << solution->objective << " " << individual->getTotalConstraintViolation() << std::endl;
    sendSolution(solution);
  });
  local_search_ = std::make_shared<ExhaustiveLocalSearch>(callbacks, neighborhood_);
}

void TspExhaustiveLocalSearch::run() {
  auto initialSolution = std::make_shared<TspIndividualStructured>(instance_.get());
  initialSolution->smartInitialize();
  initialSolution->evaluate();
  auto solution = initialSolution->convertSolution();
  portfolio_->acceptSolution(solution);
  local_search_->run(initialSolution);
}

void TspExhaustiveLocalSearch::terminate() {
  terminate_.store(true);
}

void TspExhaustiveLocalSearch::acceptSolution(
    std::shared_ptr<Solution> solution) {
  if(checkBetterSolution(solution)){
    auto individual = std::make_shared<TspIndividualStructured>(instance_.get(), solution);
    local_search_->acceptOutsideSolution(individual);
  }
}
