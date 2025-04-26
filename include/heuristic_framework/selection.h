#pragma once

#include <memory>
#include <vector>

#include "population.h"

using uint = unsigned int;

class Selection{
public:
  virtual std::vector<uint> select(std::shared_ptr<Population> &population, uint select_count) = 0;
};