#pragma once

#include <memory>
#include <mutex>
#include "simulated_annealing_fitness_diff.h"
#include "simulated_annealing_step.h"
#include "simulated_annealing_schedule.h"
#include "individual.h"
#include "callbacks.h"


class SimulatedAnnealing{
public:
private:
  std::shared_ptr<SAStep> step_;
  std::shared_ptr<SASchedule> schedule_;
  std::shared_ptr<Callbacks> callbacks_;
  std::shared_ptr<Individual> best_individual_;
  std::shared_ptr<Individual> outside_solution_;
  std::recursive_mutex outside_solution_mutex_;

  bool checkBetterSolution(const std::shared_ptr<Individual> &individual);
  bool checkOutsideSolution();

public:
  SimulatedAnnealing(const std::shared_ptr<Callbacks> &callbacks, const std::shared_ptr<SAStep> &step, const std::shared_ptr<SASchedule> &schedule);
  void acceptOutsideSolution(const std::shared_ptr<Individual> &individual);
  void run(const std::shared_ptr<Individual> &initialSolution);
};