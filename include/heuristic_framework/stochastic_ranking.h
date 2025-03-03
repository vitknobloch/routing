//
// Created by knoblvit on 3.3.25.
//

#pragma once

#include "callbacks.h"
#include "crossover.h"
#include "mutation.h"
#include "population_stochastic_ranking.h"
#include "truncation_replacement.h"
#include "tournament_selection.h"

#include <memory>
#include <mutex>
#include <random>

class StochasticRanking{
private:
  std::shared_ptr<Mutation> mutation_;
  std::shared_ptr<TournamentSelection> selection_;
  std::shared_ptr<Crossover> crossover_;
  std::shared_ptr<Callbacks> callbacks_;
  std::shared_ptr<PopulationStochasticRanking> population_;
  std::shared_ptr<Individual> best_individual_;
  std::shared_ptr<Individual> outside_solution_;
  std::recursive_mutex outside_solution_mutex_;
  std::mt19937 gen_;
  std::uniform_real_distribution<double> dist_;

  bool checkBetterSolution(const std::shared_ptr<Individual> &individual);
  bool checkOutsideSolution(const std::shared_ptr<PopulationStochasticRanking> &population);
  bool getRandomBool(const double &success_rate);

public:
  StochasticRanking(
      const std::shared_ptr<Callbacks> &callbacks,
      const std::shared_ptr<Mutation> &mutation,
      const std::shared_ptr<Crossover> &crossover,
      uint tournament_size
  );

  void acceptOutsideSolution(const std::shared_ptr<Individual> &individual);
  void run(const std::shared_ptr<PopulationStochasticRanking> &initialPopulation);
};
