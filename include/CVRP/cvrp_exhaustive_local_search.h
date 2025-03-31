//
// Created by knoblvit on 31.3.25.
//

#pragma once

#include "CVRP/cvrp_structured_individual.h"
#include "common/heuristic.h"
#include "heuristic_framework/exhaustive_local_search.h"
#include <atomic>

class CvrpExhaustiveLocalSearch : public Heuristic{
private:
  std::shared_ptr<Solution> best_solution_;
  std::shared_ptr<RoutingInstance> instance_;
  HeuristicPortfolio * portfolio_;
  std::atomic<bool> terminate_;
  std::shared_ptr<Neighborhood> neighborhood_;
  std::recursive_mutex solution_mutex_;
  std::shared_ptr<ExhaustiveLocalSearch> local_search_;

  void sendSolution(const std::shared_ptr<Solution> &solution);
  bool checkBetterSolution(const std::shared_ptr<Solution> &solution);

public:
  CvrpExhaustiveLocalSearch(const std::shared_ptr<RoutingInstance> &instance, const std::shared_ptr<Neighborhood> &neighborhood);
  void initialize(HeuristicPortfolio *portfolio) override;
  void run() override;
  void terminate() override;
  void acceptSolution(std::shared_ptr<Solution> solution) override;
};