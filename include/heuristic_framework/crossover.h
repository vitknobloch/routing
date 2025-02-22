//
// Created by knoblvit on 20.2.25.
//

#pragma once

#include "population.h"
#include <vector>

class Crossover{
public:
  virtual std::shared_ptr<Population> crossover(const std::shared_ptr<Population> &population, const std::vector<int> &parents);
};
