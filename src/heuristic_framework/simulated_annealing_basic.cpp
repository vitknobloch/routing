//
// Created by knoblvit on 15.4.25.
//
#include "heuristic_framework/simulated_annealing_basic.h"
#include <cassert>
SimulatedAnnealingBasic::SimulatedAnnealingBasic(
    const std::shared_ptr<Callbacks> &callbacks,
    const std::shared_ptr<SABasicStep> &step,
    const std::shared_ptr<SABasicSchedule> &schedule) {
  callbacks_ = callbacks;
  step_ = step;
  schedule_ = schedule;
  assert(callbacks != nullptr);
  assert(step != nullptr);
  assert(schedule != nullptr);
}

void SimulatedAnnealingBasic::run(
    const std::shared_ptr<Individual> &initialSolution) {
  std::shared_ptr<Individual> solution = initialSolution->deepcopy();
  solution->evaluate();
  best_individual_ = solution->deepcopy();

  while(!callbacks_->shouldTerminate()){
    if(checkOutsideSolution()){
      solution = best_individual_->deepcopy();
    }

    StepResult stepResult = step_->step(solution, schedule_);
    solution->evaluate();
    schedule_->registerResult(stepResult);
    checkBetterSolution(solution);
  }
}

void SimulatedAnnealingBasic::acceptOutsideSolution(
    const std::shared_ptr<Individual> &individual) {
  std::lock_guard<std::recursive_mutex> lock(outside_solution_mutex_);
  if(outside_solution_ == nullptr || individual->betterThan(outside_solution_)){
    outside_solution_ = individual;
  }
}

bool SimulatedAnnealingBasic::checkOutsideSolution() {
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

bool SimulatedAnnealingBasic::checkBetterSolution(
    const std::shared_ptr<Individual> &individual) {
  if(best_individual_ == nullptr || individual->betterThan(best_individual_)){
    best_individual_ = individual->deepcopy();
    callbacks_->newBestSolution(individual);
    return true;
  }
  return false;
}
