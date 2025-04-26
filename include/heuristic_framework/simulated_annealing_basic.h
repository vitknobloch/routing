/*
 * Acceptance does not depend on fitness value of new individual only on the wanted acceptance rate in time.
 * This is done to make use of the fast functions determining whether the new solution is better/worse
 * without actually creating and evaluating it.
 * */

#pragma once

#include <memory>
#include "mutex"
#include "simulated_annealing_basic_step.h"
#include "simulated_annealing_basic_schedule.h"
#include "individual.h"
#include "callbacks.h"

class SimulatedAnnealingBasic{
  std::shared_ptr<SABasicStep> step_;
  std::shared_ptr<SABasicSchedule> schedule_;
  std::shared_ptr<Callbacks> callbacks_;
  std::shared_ptr<Individual> best_individual_;
  std::shared_ptr<Individual> outside_solution_;
  std::recursive_mutex outside_solution_mutex_;

  bool checkBetterSolution(const std::shared_ptr<Individual> &individual);
  bool checkOutsideSolution();

public:
  SimulatedAnnealingBasic(const std::shared_ptr<Callbacks> &callbacks, const std::shared_ptr<SABasicStep> &step, const std::shared_ptr<SABasicSchedule> &schedule);
  void acceptOutsideSolution(const std::shared_ptr<Individual> &individual);
  void run(const std::shared_ptr<Individual> &initialSolution);
};