//
// Created by knoblvit on 20.2.25.
//

#pragma once

#include <vector>
#include <functional>
#include "individual.h"

class Callbacks{
private:
  std::vector<std::function<void(const std::shared_ptr<Individual> &)>> new_best_solution_callbacks_;
  std::function<bool()> terminate_;
public:
  Callbacks();
  void newBestSolution(const std::shared_ptr<Individual> &);
  bool shouldTerminate();

  void addNewBestSolutionCallback(const std::function<void(const std::shared_ptr<Individual> &)>& function);
  void setTerminationCondition(std::function<bool()>);
};