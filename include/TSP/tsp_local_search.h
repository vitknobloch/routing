#pragma once

#include "TSP/tsp_individual.h"
#include "common/heuristic.h"
#include "heuristic_framework/stochastic_local_search.h"
#include <atomic>

class TspLocalSearch : public Heuristic{
private:
  std::shared_ptr<Solution> best_solution_;
  std::shared_ptr<RoutingInstance> instance_;
  const uint *matrix_;
  HeuristicPortfolio *portfolio_;
  std::atomic<bool> terminate_;
  std::shared_ptr<Mutation> mutation_;
  std::recursive_mutex solution_mutex_;
  std::shared_ptr<StochasticLocalSearch> local_search_;

  void sendSolution(const std::shared_ptr<Solution>& solution);
  bool checkBetterSolution(const std::shared_ptr<Solution> &solution);

public:
  explicit TspLocalSearch(const std::shared_ptr<RoutingInstance> &instance, const std::shared_ptr<Mutation> &mutation);
  void initialize(HeuristicPortfolio *portfolio) override;
  void run() override;
  void terminate() override;
  void acceptSolution(std::shared_ptr<Solution>) override;
};
