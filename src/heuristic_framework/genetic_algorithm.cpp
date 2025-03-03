//
// Created by knoblvit on 2.3.25.
//
#include "heuristic_framework/genetic_algorithm.h"
#include <cassert>

GeneticAlgorithm::GeneticAlgorithm(
    const std::shared_ptr<Callbacks> &callbacks,
    const std::shared_ptr<Mutation> &mutation,
    const std::shared_ptr<Selection> &selection,
    const std::shared_ptr<Crossover> &crossover,
    const std::shared_ptr<Replacement> &replacement) {
  callbacks_ = callbacks;
  mutation_ = mutation;
  selection_ = selection;
  crossover_ = crossover;
  replacement_ = replacement;
  std::random_device rand;
  gen_ = std::mt19937(rand());
  dist_ = std::uniform_real_distribution<double>(0, 1);

  assert(callbacks != nullptr);
  assert(mutation != nullptr);
  assert(selection != nullptr);
  assert(crossover != nullptr);
  assert(replacement != nullptr);
}

void GeneticAlgorithm::run(
    const std::shared_ptr<Population> &initialPopulation) {
  population_ = initialPopulation;
  population_->evaluate();
  best_individual_ = population_->getBestIndividual();

  while(!callbacks_->shouldTerminate()){
    auto select_size = (int)((double)population_->size() * crossover_->getCrossoverRate());
    auto selection = selection_->select(population_, select_size);
    auto child_pop = crossover_->crossover(population_, selection);
    for(uint i = 0; i < child_pop->size(); i++){
      if(getRandomBool(mutation_->getMutationRate())){
        const auto &individual = child_pop->getIndividual(i);
        mutation_->mutate(individual);
        individual->resetEvaluated();
      }
    }
    checkOutsideSolution(child_pop);
    child_pop->evaluate();
    if(checkBetterSolution(child_pop->getBestIndividual())){
      callbacks_->newBestSolution(child_pop->getBestIndividual());
    }
    population_ = replacement_->replacementFunction(population_, child_pop, population_->size());
  }
}

bool GeneticAlgorithm::checkBetterSolution(
    const std::shared_ptr<Individual> &individual) {
  if(best_individual_ == nullptr || individual->betterThan(best_individual_)){
    best_individual_ = individual->deepcopy();
    callbacks_->newBestSolution(individual);
    return true;
  }
  return false;
}

bool GeneticAlgorithm::checkOutsideSolution(const std::shared_ptr<Population> &population) {
  if(outside_solution_ == nullptr)
    return false;
  std::lock_guard<std::recursive_mutex> lock(outside_solution_mutex_);
  if(outside_solution_ == nullptr)
    return false;
  if(best_individual_ == nullptr || outside_solution_->betterThan(best_individual_)){
    best_individual_ = outside_solution_;
  }
  if(population_ == nullptr)
    return false;
  population_->addIndividual(outside_solution_);
  outside_solution_ = nullptr;
  return true;
}

void GeneticAlgorithm::acceptOutsideSolution(
    const std::shared_ptr<Individual> &individual) {
  std::lock_guard<std::recursive_mutex> lock(outside_solution_mutex_);
  if(outside_solution_ == nullptr || individual->betterThan(outside_solution_)){
    outside_solution_ = individual;
  }
}

inline bool GeneticAlgorithm::getRandomBool(const double &success_rate) {
  return dist_(gen_) < success_rate;
}
