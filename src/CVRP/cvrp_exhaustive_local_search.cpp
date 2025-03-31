//
// Created by knoblvit on 31.3.25.
//
#include "CVRP/cvrp_exhaustive_local_search.h"
#include <iostream>

CvrpExhaustiveLocalSearch::CvrpExhaustiveLocalSearch(
    const std::shared_ptr<RoutingInstance> &instance,
    const std::shared_ptr<Neighborhood> &neighborhood) {
  instance_ = instance;
  best_solution_ = nullptr;
  terminate_ = false;
  local_search_ = nullptr;
  neighborhood_ = neighborhood;
}

void CvrpExhaustiveLocalSearch::sendSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution == nullptr || portfolio_ == nullptr)
    return;
  portfolio_->acceptSolution(solution);
}

bool CvrpExhaustiveLocalSearch::checkBetterSolution(
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

void CvrpExhaustiveLocalSearch::initialize(HeuristicPortfolio *portfolio) {
  portfolio_ = portfolio;
  terminate_ = false;
  std::shared_ptr<Callbacks> callbacks = std::make_shared<Callbacks>();
  callbacks->setTerminationCondition([this]() -> bool {
    return terminate_;
  });
  callbacks->addNewBestSolutionCallback([this](const std::shared_ptr<Individual> &individual) {
    auto individual_ = std::static_pointer_cast<CvrpIndividualStructured>(individual);
    auto solution = individual_->convertSolution();
    //std::cerr << solution->objective << " " << individual->getTotalConstraintViolation() << std::endl;
    sendSolution(solution);
  });
  local_search_ = std::make_shared<ExhaustiveLocalSearch>(callbacks, neighborhood_);
}

void CvrpExhaustiveLocalSearch::terminate() {
  terminate_.store(true);
}
void CvrpExhaustiveLocalSearch::acceptSolution(
    std::shared_ptr<Solution> solution) {
  if(checkBetterSolution(solution)){
    auto individual = std::make_shared<CvrpIndividualStructured>(instance_.get(), solution);
    local_search_->acceptOutsideSolution(individual);
  }
}

void CvrpExhaustiveLocalSearch::run() {
  auto initialSolution = std::make_shared<CvrpIndividualStructured>(instance_.get());
  initialSolution->smartInitialize();
  initialSolution->evaluate();
  auto solution = initialSolution->convertSolution();
  portfolio_->acceptSolution(solution);
  local_search_->run(initialSolution);
}
