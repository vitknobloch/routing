#pragma once

class Heuristic;

#include "heuristic.h"
#include "logger.h"
#include "solution.h"
#include <memory>
#include <mutex>
#include <vector>

class HeuristicPortfolio{
private:
  /** Used improving heuristics running throughout the calculation */
  std::vector<std::shared_ptr<Heuristic>> improving_heuristics_;

  /** Used constructive heuristics ran once in the beggining of calculation*/
  std::vector<std::shared_ptr<Heuristic>> constructive_heuristics_;

  std::shared_ptr<ObjectiveValueLogger> logger_;

  /** Current best-so-far solution */
  std::shared_ptr<Solution> best_solution_;
  std::mutex solution_lock_;

  /** Sends current best-so-far solution to stdout and to the improving heuristics */
  void sendSolution();

  void initializeThreads();
  void runThreads();

public:
  HeuristicPortfolio();
  void addImprovingHeuristic(const std::shared_ptr<Heuristic>& heuristic);
  void addConstructiveHeuristic(const std::shared_ptr<Heuristic>& heuristic);
  void setLogger(const std::shared_ptr<ObjectiveValueLogger> &logger);
  void start();
  void terminate();

  /** Receives new solution, checks if it is BSF solution and sends it out if it is */
  void acceptSolution(const std::shared_ptr<Solution>& solution);
};
