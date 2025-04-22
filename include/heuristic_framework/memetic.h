//
// Created by knoblvit on 20.4.25.
//

#pragma once
#include "callbacks.h"
#include "crossover.h"
#include "individual.h"
#include "mutation.h"
#include "neighborhood.h"
#include "population.h"
#include "replacement.h"
#include "selection.h"
#include <memory>
#include <mutex>
#include <random>

class MemeticAlgorithm{
private:
  std::shared_ptr<Neighborhood> neighborhood_;
  std::shared_ptr<Mutation> mutation_;
  std::shared_ptr<Population> population_;
  std::shared_ptr<Callbacks> callbacks_;
  std::shared_ptr<Selection> selection_;
  std::shared_ptr<Crossover> crossover_;
  std::shared_ptr<Replacement> replacement_;

  std::shared_ptr<Individual> best_individual_;
  std::shared_ptr<Individual> outside_solution_;
  std::recursive_mutex outside_solution_mutex_;

  std::random_device rand_;
  std::mt19937 gen_;
  std::uniform_real_distribution<double> dist_;

  bool checkBetterSolution(const std::shared_ptr<Individual> &individual);
  bool checkOutsideSolution();
  bool getRandomBool(const double &success_rate);

public:
  MemeticAlgorithm(
      const std::shared_ptr<Callbacks> &callbacks,
      const std::shared_ptr<Neighborhood> &neighborhood,
      const std::shared_ptr<Mutation> &mutation,
      const std::shared_ptr<Selection> &selection,
      const std::shared_ptr<Crossover> &crossover,
      const std::shared_ptr<Replacement> &replacement
      );

  void acceptOutsideSolution(const std::shared_ptr<Individual> &individual);
  void run(const std::shared_ptr<Population> &initialPopulation);
};
