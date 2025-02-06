//
// Created by knoblvit on 6.2.25.
//

#pragma once
#include <string>
#include "nlohmann/json.hpp"
#include "solution.h"

class SolutionSerializer{
public:
  virtual std::string serializeSolution(std::shared_ptr<Solution> &solution) = 0;
  virtual std::shared_ptr<Solution> parseSolution(std::string solution_string) = 0;
};
