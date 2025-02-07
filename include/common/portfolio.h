//
// Created by knoblvit on 6.2.25.
//

#pragma once

#include "heuristic.h"
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

  /** Current best-so-far solution */
  std::shared_ptr<Solution> best_solution_;
  std::mutex solution_lock_;

  /** Sends current best-so-far solution to stdout and to the improving heuristics */
  void sendSolution();

  void startThread(std::shared_ptr<Heuristic> heuristic);

public:
  HeuristicPortfolio();
  void addImprovingHeuristic(const std::shared_ptr<Heuristic>& heuristic);
  void addConstructiveHeuristic(const std::shared_ptr<Heuristic>& heuristic);
  void start();
  void terminate();

  /** Receives new solution, checks if it is BSF solution and sends it out if it is */
  void acceptSolution(std::shared_ptr<Solution> solution);
};
