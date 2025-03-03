//
// Created by knoblvit on 2.3.25.
//
#include "TSP/tsp_genetic_algorithm.h"
#include <cassert>
#include <iostream>

TspGeneticAlgorithm::TspGeneticAlgorithm(
    const std::shared_ptr<RoutingInstance> &instance,
    const std::shared_ptr<Mutation> &mutation,
    const std::shared_ptr<Selection> &selection,
    const std::shared_ptr<Crossover> &crossover,
    const std::shared_ptr<Replacement> &replacement,
    uint population_size) {
  instance_ = instance;
  best_solution_ = nullptr;
  terminate_ = false;
  matrix_ = instance->getMatrix().get();
  genetic_algorithm_ = nullptr;
  mutation_ = mutation;
  selection_ = selection;
  crossover_ = crossover;
  replacement_ = replacement;
  population_size_ = population_size;
}

void TspGeneticAlgorithm::run() {
  std::random_device rand;
  std::mt19937 gen(rand());
  std::shared_ptr<TspIndividual> initial_individual = std::make_shared<TspIndividual>(instance_.get());
  initial_individual->initializeNearestNeighbor();
  initial_individual->evaluate();
  auto solution = initial_individual->convertSolution();
  if(checkBetterSolution(solution))
    portfolio_->acceptSolution(solution);

  auto initial_population = std::make_shared<Population>();
  initial_population->addIndividual(initial_individual);

  for(uint i = 1; i < population_size_; i++){
    auto individual = std::make_shared<TspIndividual>(instance_.get());
    individual->initialize();
    std::shuffle(individual->data().begin(), individual->data().end(), gen);
    individual->evaluate();
    initial_population->addIndividual(individual);
  }

  assert(genetic_algorithm_ != nullptr);
  genetic_algorithm_->run(initial_population);
}

void TspGeneticAlgorithm::initialize(HeuristicPortfolio *portfolio) {
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
  genetic_algorithm_ = std::make_shared<GeneticAlgorithm>(callbacks, mutation_, selection_, crossover_, replacement_);
}

void TspGeneticAlgorithm::sendSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution == nullptr || portfolio_ == nullptr)
    return;
  portfolio_->acceptSolution(solution);
}

bool TspGeneticAlgorithm::checkBetterSolution(
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

void TspGeneticAlgorithm::acceptSolution(std::shared_ptr<Solution> solution) {
  if(checkBetterSolution(solution)){
    auto individual = std::make_shared<TspIndividual>(instance_.get(), solution);
    genetic_algorithm_->acceptOutsideSolution(individual);
  }
}

void TspGeneticAlgorithm::terminate() {
  terminate_.store(true);
}
