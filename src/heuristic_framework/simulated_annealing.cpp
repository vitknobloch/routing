#include "heuristic_framework/simulated_annealing.h"
#include <cassert>

SimulatedAnnealing::SimulatedAnnealing(
    const std::shared_ptr<Callbacks> &callbacks,
    const std::shared_ptr<SAStep> &step,
    const std::shared_ptr<SASchedule> &schedule) {
  callbacks_ = callbacks;
  step_ = step;
  schedule_ = schedule;
  assert(callbacks != nullptr);
  assert(step != nullptr);
  assert(schedule != nullptr);
}

void SimulatedAnnealing::run(
    const std::shared_ptr<Individual> &initialSolution) {
  schedule_->reset();
  std::shared_ptr<Individual> solution = initialSolution->deepcopy();
  solution->evaluate();
  best_individual_ = solution->deepcopy();

  while(!callbacks_->shouldTerminate()){
    if(checkOutsideSolution()){
      solution = best_individual_->deepcopy();
    }
    StepResult stepResult = step_->step(solution, schedule_, best_individual_);
    solution->evaluate();
    schedule_->registerResult(stepResult);
    checkBetterSolution(solution);
  }
}

void SimulatedAnnealing::acceptOutsideSolution(
    const std::shared_ptr<Individual> &individual) {
  std::lock_guard<std::recursive_mutex> lock(outside_solution_mutex_);
  if(outside_solution_ == nullptr || individual->betterThan(outside_solution_)){
    outside_solution_ = individual;
  }
}
bool SimulatedAnnealing::checkOutsideSolution() {
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

bool SimulatedAnnealing::checkBetterSolution(
    const std::shared_ptr<Individual> &individual) {
  if(best_individual_ == nullptr || individual->betterThan(best_individual_)){
    best_individual_ = individual->deepcopy();
    callbacks_->newBestSolution(individual);
    return true;
  }
  return false;
}
