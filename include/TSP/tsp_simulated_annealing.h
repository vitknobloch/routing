#pragma once
#include "heuristic_framework/simulated_annealing.h"
#include "heuristic_framework/simulated_annealing_schedule.h"
#include "TSP/tsp_individual_structured.h"
#include "common/heuristic.h"
#include <atomic>

class TspSimulatedAnnealing : public Heuristic{
private:
  std::shared_ptr<Solution> best_solution_;
  std::shared_ptr<RoutingInstance> instance_;
  HeuristicPortfolio *portfolio_;
  std::atomic<bool> terminate_;
  std::recursive_mutex solution_mutex_;
  std::shared_ptr<SAStep> step_;
  std::shared_ptr<SASchedule> schedule_;
  std::shared_ptr<SimulatedAnnealing> simulated_annealing_;

  void sendSolution(const std::shared_ptr<Solution> &solution);
  bool checkBetterSolution(const std::shared_ptr<Solution> &solution);

public:
  TspSimulatedAnnealing(const std::shared_ptr<RoutingInstance> &instance, const std::shared_ptr<SAStep> &step, const std::shared_ptr<SASchedule> schedule);
  void initialize(HeuristicPortfolio *portfolio) override;
  void run() override;
  void terminate() override;
  void acceptSolution(std::shared_ptr<Solution> solution) override;

};