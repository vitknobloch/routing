//
// Created by knoblvit on 26.4.25.
//
#include "VRP-TW/vrptw_memetic.h"
#include <iostream>

VrptwMemetic::VrptwMemetic(const std::shared_ptr<RoutingInstance> &instance,
                           const std::shared_ptr<Neighborhood> &neighborhood,
                           const std::shared_ptr<Mutation> &mutation,
                           const std::shared_ptr<Selection> &selection,
                           const std::shared_ptr<Crossover> &crossover,
                           const std::shared_ptr<Replacement> &replacement,
                           uint population_size) {
  instance_ = instance;
  neighborhood_ = neighborhood;
  mutation_ = mutation;
  selection_ = selection;
  crossover_ = crossover;
  replacement_ = replacement;
  population_size_ = population_size;
  best_solution_ = nullptr;
  terminate_ = false;
  memetic_algorithm_ = nullptr;
  portfolio_ = nullptr;
}

void VrptwMemetic::sendSolution(const std::shared_ptr<Solution> &solution) {
  if(solution == nullptr || portfolio_ == nullptr)
    return;
  portfolio_->acceptSolution(solution);
}

bool VrptwMemetic::checkBetterSolution(
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

void VrptwMemetic::initialize(HeuristicPortfolio *portfolio) {
  portfolio_ = portfolio;
  terminate_ = false;
  std::shared_ptr<Callbacks> callbacks = std::make_shared<Callbacks>();
  callbacks->setTerminationCondition([this]() -> bool {
    return terminate_;
  });
  callbacks->addNewBestSolutionCallback([this](const std::shared_ptr<Individual> &individual) {
    auto individual_ = std::static_pointer_cast<VrptwIndividualStructured>(individual);
    auto solution = individual_->convertSolution();
    sendSolution(solution);
  });
  memetic_algorithm_ = std::make_shared<MemeticAlgorithm>(callbacks, neighborhood_, mutation_, selection_, crossover_, replacement_);
}

void VrptwMemetic::run() {
  auto initialPopulation = std::make_shared<Population>();
  for(uint i = 0; i < population_size_; i++){
    auto solution = std::make_shared<VrptwIndividualStructured>(instance_.get());
    solution->smartInitialize();
    initialPopulation->addIndividual(solution);
  }
  initialPopulation->evaluate();
  const auto &best_individual = std::static_pointer_cast<VrptwIndividualStructured>(initialPopulation->getBestIndividual());
  best_solution_ = best_individual->convertSolution();
  portfolio_->acceptSolution(best_solution_);
  memetic_algorithm_->run(initialPopulation);
}

void VrptwMemetic::terminate() {
  terminate_.store(true);
}
void VrptwMemetic::acceptSolution(std::shared_ptr<Solution> solution) {
  if(checkBetterSolution(solution)){
    auto individual = std::make_shared<VrptwIndividualStructured>(instance_.get(), solution);
    memetic_algorithm_->acceptOutsideSolution(individual);
  }
}
