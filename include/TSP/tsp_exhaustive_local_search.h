#pragma once

#include "TSP/tsp_individual_structured.h"
#include "common/heuristic.h"
#include "heuristic_framework/exhaustive_local_search.h"
#include <atomic>

class TspExhaustiveLocalSearch : public Heuristic{
private:
  std::shared_ptr<Solution> best_solution_;
  std::shared_ptr<RoutingInstance> instance_;
  HeuristicPortfolio *portfolio_;
  std::atomic<bool> terminate_;
  std::shared_ptr<Neighborhood> neighborhood_;
  std::recursive_mutex solution_mutex_;
  std::shared_ptr<ExhaustiveLocalSearch> local_search_;

  void sendSolution(const std::shared_ptr<Solution> &solution);
  bool checkBetterSolution(const std::shared_ptr<Solution> &solution);

public:
  TspExhaustiveLocalSearch(const std::shared_ptr<RoutingInstance> &instance, const std::shared_ptr<Neighborhood> &neighborhood);
  void initialize(HeuristicPortfolio *portfolio) override;
  void run() override;
  void terminate() override;
  void acceptSolution(std::shared_ptr<Solution> solution) override;
};