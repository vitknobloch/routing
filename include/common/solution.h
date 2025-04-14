//
// Created by knoblvit on 6.2.25.
//

#pragma once
#include <list>

struct SolutionNode{
  int idx = -1;
  int start_time = -1;
  int end_time = -1;

  SolutionNode() = default;
  SolutionNode(int idx, int start_time, int end_time){
    this->idx = idx;
    this->start_time = start_time;
    this->end_time = end_time;
  }
};

struct SolutionRoute{
  std::list<SolutionNode> route_nodes;
  int travel_time = -1;
  int demand = -1;
  int end_time = -1;

};

struct Solution {
  std::list<SolutionRoute> routes;
  int used_vehicles = 0;
  int travel_time_sum = -1;
  int end_time_sum = -1;
  int objective = -1;
  bool feasible = true;

  [[nodiscard]] bool betterThan(const Solution &other) const{
    if(!feasible && other.feasible)
      return false;
    if(used_vehicles > other.used_vehicles)
      return false;
    else if(used_vehicles < other.used_vehicles)
      return true;
    return objective < other.objective;
  }
};