//
// Created by knoblvit on 31.3.25.
//
#include "heuristic_framework/exhaustive_local_search.h"
#include <cassert>
#include <iostream>

ExhaustiveLocalSearch::ExhaustiveLocalSearch(
    const std::shared_ptr<Callbacks> &callbacks,
    const std::shared_ptr<Neighborhood> &neighborhood) {
  callbacks_ = callbacks;
  neighborhood_ = neighborhood;
  assert(callbacks != nullptr);
  assert(neighborhood != nullptr);
}

void ExhaustiveLocalSearch::run(
    const std::shared_ptr<Individual> &initialSolution) {
  std::shared_ptr<Individual> solution = initialSolution->deepcopy();
  solution->evaluate();
  best_individual_ = solution->deepcopy();

  while(!callbacks_->shouldTerminate()){
    if(checkOutsideSolution()){
      solution = best_individual_->deepcopy();
    }
    Neighborhood::SearchResult result = neighborhood_->search(solution);
    if(result == Neighborhood::SearchResult::IMPROVED){
      // send solution if it is the best so far
      checkBetterSolution(solution);
    }else if(result == Neighborhood::SearchResult::UNIMPROVED){
      // nothing to do
    }else if(result == Neighborhood::SearchResult::EXHAUSTED){
      // restart search
      std::cerr << "Restarted search" << std::endl;
      neighborhood_->reset(solution);
      solution->smartInitialize();
      solution->evaluate();
    }
  }
}

void ExhaustiveLocalSearch::acceptOutsideSolution(
    const std::shared_ptr<Individual> &individual) {
  std::lock_guard<std::recursive_mutex> lock(outside_solution_mutex_);
  if(outside_solution_ == nullptr || individual->betterThan(outside_solution_)){
    outside_solution_ = individual;
  }
}

bool ExhaustiveLocalSearch::checkOutsideSolution() {
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

bool ExhaustiveLocalSearch::checkBetterSolution(
    const std::shared_ptr<Individual> &individual) {
  if(best_individual_ == nullptr || individual->betterThan(best_individual_)){
    best_individual_ = individual->deepcopy();
    callbacks_->newBestSolution(individual);
    return true;
  }
  return false;
}
