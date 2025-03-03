//
// Created by knoblvit on 2.3.25.
//

#pragma once

#include "TSP/tsp_individual.h"
#include "heuristic_framework/genetic_algorithm.h"
#include "common/heuristic.h"
#include <atomic>

class TspGeneticAlgorithm : public Heuristic{
private:
  std::shared_ptr<Solution> best_solution_;
  std::shared_ptr<RoutingInstance> instance_;
  const uint *matrix_;
  HeuristicPortfolio *portfolio_;
  std::atomic<bool> terminate_;

  std::shared_ptr<Mutation> mutation_;
  std::shared_ptr<Selection> selection_;
  std::shared_ptr<Crossover> crossover_;
  std::shared_ptr<Replacement> replacement_;
  uint population_size_;

  std::recursive_mutex solution_mutex_;
  std::shared_ptr<GeneticAlgorithm> genetic_algorithm_;

  void sendSolution(const std::shared_ptr<Solution> &solution);
  bool checkBetterSolution(const std::shared_ptr<Solution> &solution);

public:
  TspGeneticAlgorithm(
    const std::shared_ptr<RoutingInstance> &instance,
    const std::shared_ptr<Mutation> &mutation,
    const std::shared_ptr<Selection> &selection,
    const std::shared_ptr<Crossover> &crossover,
    const std::shared_ptr<Replacement> &replacement,
    uint population_size
    );

  void initialize(HeuristicPortfolio * portfolio) override;
  void run() override;
  void terminate() override;
  void acceptSolution(std::shared_ptr<Solution>) override;


public:


};
