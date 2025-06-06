#pragma once

#include <memory>
#include <mutex>
#include "mutation.h"
#include "individual.h"
#include "callbacks.h"

class StochasticLocalSearch {
private:
  std::shared_ptr<Mutation> mutation_;
  std::shared_ptr<Callbacks> callbacks_;
  std::shared_ptr<Individual> best_individual_;
  std::shared_ptr<Individual> outside_solution_;
  std::recursive_mutex outside_solution_mutex_;

  bool checkBetterSolution(const std::shared_ptr<Individual> &individual);
  bool checkOutsideSolution();

public:
  StochasticLocalSearch(const std::shared_ptr<Callbacks> &callbacks, const std::shared_ptr<Mutation> &mutation);
  void acceptOutsideSolution(const std::shared_ptr<Individual> &individual);
  void run(const std::shared_ptr<Individual> &initialSolution);
};
