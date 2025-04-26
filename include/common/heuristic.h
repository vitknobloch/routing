#pragma once

class HeuristicPortfolio;

#include "portfolio.h"
#include "solution.h"
#include <memory>

/** Parent class for heuristics, heuristics need to implement these functions*/
class Heuristic {
public:
  /** Takes a new best-so-far solution and asynchronosly adds it to the heuristic population for refinement */
  virtual void acceptSolution(std::shared_ptr<Solution>) = 0;
  /** Prepare data for run of the heuristic */
  virtual void initialize(HeuristicPortfolio *portfolio) = 0;
  /** Start the heuristic */
  virtual void run() = 0;
  /** Asynchronously set termination condition so that the heuristic exits after the current iteration */
  virtual void terminate() = 0;

  virtual ~Heuristic() = default;
};
