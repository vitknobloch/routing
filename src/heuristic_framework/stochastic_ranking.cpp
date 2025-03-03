//
// Created by knoblvit on 3.3.25.
//
#include "heuristic_framework/stochastic_ranking.h"
#include <cassert>

StochasticRanking::StochasticRanking(
    const std::shared_ptr<Callbacks> &callbacks,
    const std::shared_ptr<Mutation> &mutation,
    const std::shared_ptr<Crossover> &crossover, uint tournament_size) {
  mutation_ = mutation;
  selection_ = std::make_shared<TournamentSelection>(tournament_size);
  crossover_ = crossover;
  replacement_ = std::make_shared<TruncationReplacement>();
  callbacks_ = callbacks;
  population_ = std::make_shared<PopulationStochasticRanking>(0.5);
  best_individual_ = nullptr;
  outside_solution_ = nullptr;
  std::random_device rand;
  gen_ = std::mt19937(rand());
  dist_ = std::uniform_real_distribution<double>(0,1);

  assert(mutation != nullptr);
  assert(callbacks != nullptr);
  assert(crossover != nullptr);
}

void StochasticRanking::run(
    const std::shared_ptr<PopulationStochasticRanking> &initialPopulation) {
  population_ = initialPopulation;
  population_->evaluate();
  best_individual_ = population_->getBestIndividual();

  while(!callbacks_->shouldTerminate()){
    auto select_size = (int)((double)population_->size() * crossover_->getCrossoverRate());
    auto population_cast = std::static_pointer_cast<Population>(population_);
    auto selection = selection_->select(population_cast, select_size);
    auto child_pop_cast = crossover_->crossover(population_, selection);
    auto child_pop = std::static_pointer_cast<PopulationStochasticRanking>(child_pop_cast);

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
    population_ = std::static_pointer_cast<PopulationStochasticRanking>(replacement_->replacementFunction(population_, child_pop, population_->size()));
  }
}

bool StochasticRanking::checkBetterSolution(
    const std::shared_ptr<Individual> &individual) {
  if(best_individual_ == nullptr || individual->betterThan(best_individual_)){
    best_individual_ = individual->deepcopy();
    callbacks_->newBestSolution(individual);
    return true;
  }
  return false;
}

bool StochasticRanking::checkOutsideSolution(
    const std::shared_ptr<PopulationStochasticRanking> &population) {
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

inline bool StochasticRanking::getRandomBool(const double &success_rate) {
  return dist_(gen_) < success_rate;
}

void StochasticRanking::acceptOutsideSolution(
    const std::shared_ptr<Individual> &individual) {
  std::lock_guard<std::recursive_mutex> lock(outside_solution_mutex_);
  if(outside_solution_ == nullptr || individual->betterThan(outside_solution_)){
    outside_solution_ = individual;
  }
}
