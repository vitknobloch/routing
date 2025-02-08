//
// Created by knoblvit on 7.2.25.
//
#include "common/serializer.h"
#include <sstream>

using JSON = nlohmann::json;

std::shared_ptr<Solution>
SolutionSerializer::parseSolution(std::string solution_string) {
  auto solution = std::make_shared<Solution>();

  JSON solution_json;
  std::istringstream stream(solution_string);
  stream >> solution_json;

  solution->objective = solution_json.at("objective");
  solution->end_time_sum = solution_json.at("end_time_sum");
  solution->travel_time_sum = solution_json.at("travel_time_sum");
  auto routes_json = solution_json.at("routes");
  size_t route_count = routes_json.size();
  solution->routes = std::list<SolutionRoute>();
  for(uint i = 0; i < route_count; i++){
    solution->routes.push_back(parseRoute(routes_json[i]));
  }
  return solution;
}

std::string
SolutionSerializer::serializeSolution(std::shared_ptr<Solution> &solution) {
  std::vector<JSON> routes_json;
  routes_json.reserve(solution->routes.size());
  for(const auto &route : solution->routes){
    routes_json.push_back(serializeRoute(route));
  }

  JSON solution_json;
  solution_json["objective"] = solution->objective;
  solution_json["end_time_sum"] = solution->end_time_sum;
  solution_json["travel_time_sum"] = solution->travel_time_sum;
  solution_json["routes"] = routes_json;

  return solution_json.dump();
}

nlohmann::json SolutionSerializer::serializeNode(const SolutionNode &node) {
  JSON node_json;
  node_json["idx"] = node.idx;
  node_json["start_time"] = node.start_time;
  node_json["end_time"] = node.end_time;
  return node_json;
}

nlohmann::json
SolutionSerializer::serializeRoute(const SolutionRoute &route) {
  std::vector<JSON> nodes_json;
  nodes_json.reserve(route.route_nodes.size());
  for(const auto & node: route.route_nodes){
    nodes_json.push_back(serializeNode(node));
  }

  JSON route_json;
  route_json["travel_time"] = route.travel_time;
  route_json["demand"] = route.demand;
  route_json["end_time"] = route.end_time;
  route_json["route_nodes"] = nodes_json;
  return route_json;
}

SolutionNode SolutionSerializer::parseNode(const nlohmann::json &node_json) {
  SolutionNode node;
  node.idx = node_json.at("idx");
  node.start_time = node_json.at("start_time");
  node.end_time = node_json.at("end_time");
  return node;
}

SolutionRoute
SolutionSerializer::parseRoute(const nlohmann::json &route_json) {
  SolutionRoute route;
  route.demand = route_json.at("demand");
  route.end_time = route_json.at("end_time");
  route.travel_time = route_json.at("travel_time");
  route.route_nodes = std::list<SolutionNode>();

  const auto &nodes_json = route_json.at("route_nodes");
  size_t node_count = nodes_json.size();
  for(uint i = 0; i < node_count; i++){
    route.route_nodes.push_back(parseNode(nodes_json[i]));
  }
  return route;
}
