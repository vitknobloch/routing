//
// Created by knoblvit on 7.2.25.
//

#pragma once
#include "common/serializer.h"

class SolutionSerializerTSP: public SolutionSerializer{
private:
  nlohmann::json serializeNode(const SolutionNode &node);
  nlohmann::json serializeRoute(const SolutionRoute &route);

  SolutionNode parseNode(const nlohmann::json &node_json);
  SolutionRoute parseRoute(const nlohmann::json &route_json);

public:
  std::shared_ptr<Solution> parseSolution(std::string solution_string) override;
  std::string serializeSolution(std::shared_ptr<Solution> &solution) override;
};
