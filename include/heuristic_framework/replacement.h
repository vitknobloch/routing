#pragma once
#include "population.h"

class Replacement{
public:
  virtual std::shared_ptr<Population> replacementFunction(const std::shared_ptr<Population> &old_pop, const std::shared_ptr<Population> &new_pop, uint final_size) = 0;
};