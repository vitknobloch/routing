#include "heuristic_framework/stochastic_local_search.h"
#include <cassert>
#include <iostream>

StochasticLocalSearch::StochasticLocalSearch(const std::shared_ptr<Callbacks> &callbacks,
                         const std::shared_ptr<Mutation> &mutation) {
  callbacks_ = callbacks;
  mutation_ = mutation;
  assert(callbacks != nullptr);
  assert(mutation != nullptr);
}

void StochasticLocalSearch::run(const std::shared_ptr<Individual> &initialSolution) {
  std::shared_ptr<Individual> solution = initialSolution->deepcopy();
  solution->evaluate();
  best_individual_ = solution->deepcopy();

  while(!callbacks_->shouldTerminate()){
    if(checkOutsideSolution()){
      solution = best_individual_->deepcopy();
    }
    if(mutation_->isInPlace()){
      mutation_->mutate(solution);
      solution->evaluate();
      checkBetterSolution(solution);
    }
    else{
      std::shared_ptr<Individual> new_solution = solution->deepcopy();
      mutation_->mutate(new_solution);
      new_solution->evaluate();
      if(checkBetterSolution(new_solution)){
        solution = new_solution;
      }
    }
  }
}

bool StochasticLocalSearch::checkBetterSolution(
    const std::shared_ptr<Individual> &individual) {
  if(best_individual_ == nullptr || individual->betterThan(best_individual_)){
        best_individual_ = individual->deepcopy();
        callbacks_->newBestSolution(individual);
        return true;
  }
  return false;
}

void StochasticLocalSearch::acceptOutsideSolution(
    const std::shared_ptr<Individual> &individual) {
  std::lock_guard<std::recursive_mutex> lock(outside_solution_mutex_);
  if(outside_solution_ == nullptr || individual->betterThan(outside_solution_)){
    outside_solution_ = individual;
  }
}

bool StochasticLocalSearch::checkOutsideSolution() {
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
