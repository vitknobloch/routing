//
// Created by knoblvit on 10.2.25.
//

#pragma once
#include "common/routing_instance.h"

class RoutingInstance::SolomonLoader{
private:
  static void error();
  inline static uint dist(const std::pair<int, int> &n1, const std::pair<int, int> &n2);
  static void buildTransitionMatrix(RoutingInstance &instance, const std::vector<std::pair<int, int>> &node_poses);
public:
  static void loadHeader(RoutingInstance &instance, std::ifstream &file);
  static void loadNodes(RoutingInstance &instance, std::ifstream &file);

};
