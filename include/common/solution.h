//
// Created by knoblvit on 6.2.25.
//

#pragma once
#include <list>

struct SolutionNode{
  int idx = -1;
  int start_time = -1;
  int end_time = -1;
};

struct SolutionRoute{
  std::list<SolutionNode> route_nodes;
  int travel_time = -1;
  int demand = -1;
  int end_time = -1;

};

struct Solution {
  std::list<SolutionRoute> routes;
  int travel_time_sum = -1;
  int end_time_sum = -1;
  int objective = -1;
};