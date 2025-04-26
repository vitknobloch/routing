#pragma once

#include "population.h"
#include <vector>

class Crossover{
public:
  virtual std::shared_ptr<Population> crossover(const std::shared_ptr<Population> &population, const std::vector<uint> &parents) = 0;
  virtual double getCrossoverRate() = 0;
};
