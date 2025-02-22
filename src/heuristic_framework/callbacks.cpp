//
// Created by knoblvit on 22.2.25.
//

#include "heuristic_framework/callbacks.h"

#include <iostream>
#include <utility>

void Callbacks::newBestSolution(const std::shared_ptr<Individual> &solution) {
  for(const auto& callback : new_best_solution_callbacks_){
    callback(solution);
  }
}

bool Callbacks::shouldTerminate() {
  return terminate_();
}

void Callbacks::addNewBestSolutionCallback(
    const std::function<void(const std::shared_ptr<Individual> &)>& function) {
  new_best_solution_callbacks_.push_back(function);
}

void Callbacks::setTerminationCondition(std::function<bool()> function) {
  terminate_ = std::move(function);
}

Callbacks::Callbacks(): new_best_solution_callbacks_() {
  terminate_ = []() -> bool{
    std::cerr << "Termination condition not set!" << std::endl;
    return true;
  };
}
