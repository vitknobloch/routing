#pragma once
#include "heuristic_framework/simulated_annealing_basic.h"
#include "VRP-TW/vrptw_structured_individual.h"
#include "common/heuristic.h"
#include <atomic>

class VrptwSABasic : public Heuristic{
private:
  std::shared_ptr<Solution> best_solution_;
  std::shared_ptr<RoutingInstance> instance_;
  HeuristicPortfolio *portfolio_;
  std::atomic<bool> terminate_;
  std::recursive_mutex solution_mutex_;
  std::shared_ptr<SABasicStep> step_;
  std::shared_ptr<SABasicSchedule> schedule_;
  std::shared_ptr<SimulatedAnnealingBasic> sa_basic_;

  void sendSolution(const std::shared_ptr<Solution> &solution);
  bool checkBetterSolution(const std::shared_ptr<Solution> &solution);

public:
  VrptwSABasic(const std::shared_ptr<RoutingInstance> &instance, const std::shared_ptr<SABasicStep> &step, const std::shared_ptr<SABasicSchedule> schedule);
  void initialize(HeuristicPortfolio *portfolio) override;
  void run() override;
  void terminate() override;
  void acceptSolution(std::shared_ptr<Solution> solution) override;


};