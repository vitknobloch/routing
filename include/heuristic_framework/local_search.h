//
// Created by knoblvit on 20.2.25.
//

#pragma once

#include <memory>
#include <mutex>
#include "mutation.h"
#include "individual.h"
#include "callbacks.h"

class LocalSearch{
private:
  std::shared_ptr<Mutation> mutation_;
  std::shared_ptr<Callbacks> callbacks_;
  std::shared_ptr<Individual> best_individual_;


public:
  LocalSearch(const std::shared_ptr<Callbacks> &callbacks, const std::shared_ptr<Mutation> &mutation);
  void setSolutionIfBetter(const std::shared_ptr<Individual> &individual);
  void run(const std::shared_ptr<Individual> &initialSolution);


};
