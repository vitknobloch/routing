#include "common/solomon_loader.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

void RoutingInstance::SolomonLoader::error() {
  std::cerr << "Error loading instance file" << std::endl;
  exit(101);
}
void RoutingInstance::SolomonLoader::loadHeader(RoutingInstance &instance,
                                                std::ifstream &file) {
  std::string line;
  if(!std::getline(file, line))
    error();

  instance.instance_name_ = line;

  for(int i = 0; i < 4; i++)
    if(!std::getline(file, line))
      error();

  std::istringstream line_stream(line);
  line_stream >> instance.vehicle_count_;
  line_stream >> instance.vehicle_capacity_;

  for(int i = 0; i < 4; i++)
    if(!std::getline(file, line))
      error();

  instance.problem_type_ = VRPTW;
  instance.edge_weight_type_ = CEIL_2D;
  instance.edge_weight_format_ = FUNCTION;
  instance.display_data_type_ = COORD_DISPLAY;
  instance.comment_ = (std::ostringstream() << "VRP-TW instance " << instance.instance_name_).str();
  instance.depots_ = {0};
}

void RoutingInstance::SolomonLoader::loadNodes(RoutingInstance &instance,
                                               std::ifstream &file) {
  std::string line;
  std::istringstream line_stream;
  int node_count = 0;
  std::vector<std::pair<int, int>> node_coords;
  while(std::getline(file, line)){
    int idx, x_coord, y_coord, demand, ready_time, due_date, service_time;
    line_stream = std::istringstream(line);
    line_stream >> idx >> x_coord >> y_coord >> demand >> ready_time >> due_date >> service_time;
    assert(idx == node_count);
    node_count++;
    instance.nodes_.emplace_back(idx, demand, ready_time, due_date, service_time);
    node_coords.emplace_back(x_coord, y_coord);
  }

  assert((size_t)node_count == instance.nodes_.size());
  instance.node_count_ = node_count;

  buildTransitionMatrix(instance, node_coords);
}
void RoutingInstance::SolomonLoader::buildTransitionMatrix(
    RoutingInstance &instance,
    const std::vector<std::pair<int, int>> &node_poses) {
  const auto node_c = instance.nodes_.size();
  instance.matrix_ = std::shared_ptr<unsigned int[]>(new unsigned int[node_c * node_c]);
  for(size_t i = 0; i < node_c; i++){
    for(size_t j = 0; j < node_c; j++){
      instance.matrix_[i * node_c + j] = dist(node_poses[i], node_poses[j]); // NOLINT(*-narrowing-conversions)
    }
  }

  for(size_t i = 0; i < node_c; i++)
    instance.matrix_[i * node_c + i] = 0; // NOLINT(*-narrowing-conversions)
}

inline uint RoutingInstance::SolomonLoader::dist(const std::pair<int, int> &n1,
                                          const std::pair<int, int> &n2) {
  double float_dist = std::pow((double)(n1.first - n2.first), 2) + std::pow((double)(n1.second - n2.second), 2);
  float_dist = std::sqrt(float_dist);
  return (uint)ceil(float_dist);
}
