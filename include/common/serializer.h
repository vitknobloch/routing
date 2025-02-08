//
// Created by knoblvit on 6.2.25.
//

#pragma once
#include <string>
#include "nlohmann/json.hpp"
#include "solution.h"

class SolutionSerializer{
private:
  nlohmann::json serializeNode(const SolutionNode &node);
  nlohmann::json serializeRoute(const SolutionRoute &route);

  SolutionNode parseNode(const nlohmann::json &node_json);
  SolutionRoute parseRoute(const nlohmann::json &route_json);

public:
  std::string serializeSolution(std::shared_ptr<Solution> &solution);
  std::shared_ptr<Solution> parseSolution(std::string solution_string);
};
