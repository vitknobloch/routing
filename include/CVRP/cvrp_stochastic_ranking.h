#pragma once

#include "CVRP/cvrp_individual.h"
#include "heuristic_framework/stochastic_ranking.h"
#include "common/heuristic.h"
#include <atomic>

class CvrpStochasticRanking : public Heuristic{
private:
  std::shared_ptr<Solution> best_solution_;
  std::shared_ptr<RoutingInstance> instance_;
  const uint *matrix_;
  HeuristicPortfolio *portfolio_;
  std::atomic<bool> terminate_;

  std::shared_ptr<Mutation> mutation_;
  std::shared_ptr<Crossover> crossover_;
  uint population_size_;
  uint tournament_size_;
  double fitness_comp_prob_;

  std::recursive_mutex solution_mutex_;
  std::shared_ptr<StochasticRanking> stochastic_ranking_;

  void sendSolution(const std::shared_ptr<Solution> &solution);
  bool checkBetterSolution(const std::shared_ptr<Solution> &solution);

public:
  CvrpStochasticRanking(
    const std::shared_ptr<RoutingInstance> &instance,
    const std::shared_ptr<Mutation> &mutation,
    const std::shared_ptr<Crossover> &crossover,
    uint population_size,
    uint tournament_size,
    double fitness_comp_prob
    );

  void initialize(HeuristicPortfolio *portfolio) override;
  void run() override;
  void terminate() override;
  void acceptSolution(std::shared_ptr<Solution> solution) override;
};
