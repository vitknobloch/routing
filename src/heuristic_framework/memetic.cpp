//
// Created by knoblvit on 21.4.25.
//

#include "heuristic_framework/memetic.h"
#include <cassert>

MemeticAlgorithm::MemeticAlgorithm(
    const std::shared_ptr<Callbacks> &callbacks,
    const std::shared_ptr<Neighborhood> &neighborhood,
    const std::shared_ptr<Mutation> &mutation,
    const std::shared_ptr<Selection> &selection,
    const std::shared_ptr<Crossover> &crossover,
    const std::shared_ptr<Replacement> &replacement) : rand_(), gen_(rand_()), dist_(0, 1) {
  callbacks_ = callbacks;
  neighborhood_ = neighborhood;
  mutation_ = mutation;
  selection_ = selection;
  crossover_ = crossover;
  replacement_ = replacement;
  assert(callbacks != nullptr);
  assert(neighborhood != nullptr);
  assert(mutation != nullptr);
  assert(selection != nullptr);
  assert(crossover != nullptr);
  assert(replacement != nullptr);
}

bool MemeticAlgorithm::checkBetterSolution(
    const std::shared_ptr<Individual> &individual) {
  if(best_individual_ == nullptr || individual->betterThan(best_individual_)){
    best_individual_ = individual->deepcopy();
    callbacks_->newBestSolution(individual);
    return true;
  }
  return false;
}

bool MemeticAlgorithm::checkOutsideSolution() {
  if(outside_solution_ == nullptr)
    return false;
  std::lock_guard<std::recursive_mutex> lock(outside_solution_mutex_);
  if(outside_solution_ == nullptr)
    return false;
  if(best_individual_ == nullptr || outside_solution_->betterThan(best_individual_)){
    best_individual_ = outside_solution_;
    outside_solution_ = nullptr;
    return true;
  }
  outside_solution_ = nullptr;
  return false;
}

void MemeticAlgorithm::acceptOutsideSolution(
    const std::shared_ptr<Individual> &individual) {
  std::lock_guard<std::recursive_mutex> lock(outside_solution_mutex_);
  if(outside_solution_ == nullptr || individual->betterThan(outside_solution_)){
    outside_solution_ = individual;
  }
}

void MemeticAlgorithm::run(
    const std::shared_ptr<Population> &initialPopulation) {
  population_ = initialPopulation;
  population_->evaluate();
  best_individual_ = population_->getBestIndividual()->deepcopy();
  uint stable_population_size = (uint)population_->size();

  while(!callbacks_->shouldTerminate()){
    if(checkOutsideSolution()){
      // Add outside solution to population
      population_->addIndividual(best_individual_);
    }else{
      // Add a new random solution to population
      auto new_solution = population_->getIndividual(0)->deepcopy();
      new_solution->smartInitialize();
      new_solution->resetEvaluated();
      new_solution->evaluate();
      population_->addIndividual(new_solution);
      Neighborhood::SearchResult result = neighborhood_->search(new_solution);
      while(result != Neighborhood::SearchResult::EXHAUSTED){
        checkBetterSolution(new_solution);
        result = neighborhood_->search(new_solution);
      }
      checkBetterSolution(new_solution);
    }

    // Select parents and gen new population by crossover
    auto select_size = (int)((double)population_->size() * crossover_->getCrossoverRate());
    auto selection = selection_->select(population_, select_size);
    auto child_pop = crossover_->crossover(population_, selection);

    // Mutate and locally optimize new population
    for(uint i = 0; i < child_pop->size(); i++){
      const auto &individual = child_pop->getIndividual((int)i);
      if(getRandomBool(mutation_->getMutationRate())){
        mutation_->mutate(individual);
      }
      individual->evaluate();
      Neighborhood::SearchResult result = neighborhood_->search(individual);
      while(result != Neighborhood::SearchResult::EXHAUSTED){
        checkBetterSolution(individual);
        result = neighborhood_->search(individual);
      }
      checkBetterSolution(individual);
    }

    // Merge new population into population
    population_ = replacement_->replacementFunction(population_, child_pop, stable_population_size);
  }
}
bool MemeticAlgorithm::getRandomBool(const double &success_rate) {
  return dist_(gen_) < success_rate;
}
