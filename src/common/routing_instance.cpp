//
// Created by knoblvit on 9.2.25.
//
#include "common/routing_instance.h"
#include "common/tsplib_loader.h"
#include "common/solomon_loader.h"


RoutingInstance::RoutingInstance() {
  problem_type_ = TSP;
  edge_weight_type_ = EUC_2D;
  edge_weight_format_ = FUNCTION;
  display_data_type_ = NO_DISPLAY;
  vehicle_capacity_ = -1;
  vehicle_count_ = 1;
  node_count_ = 0;
}

void RoutingInstance::loadTSPlibInstance(const char *filename) {
  std::ifstream file(filename);
  if(!file.is_open()){
    std::cerr << "Error opening file: " << filename << std::endl;
    exit(100);
  }

  TSPlibLoader::loadHeader(*this, file);
  uint sum_demands = 0;
  for(const auto &node : nodes_){
    sum_demands += node.demand;
  }

  const std::regex pattern(R"((.*/)?A-n\d+-k(\d+)\.vrp)");

  std::smatch match;

  // Check for str1
  std::string filename_(filename);
  if (std::regex_match(filename_, match, pattern)) {
    vehicle_count_ = std::stoi(match[2]);
  } else {
    vehicle_count_ = (int)std::round((1.5 * (double)sum_demands) / (double)vehicle_capacity_);
  }
}

void RoutingInstance::loadSolomonInstance(const char *filename) {
  std::ifstream file(filename);
  if(!file.is_open()){
    std::cerr << "Error opening file: " << filename << std::endl;
    exit(100);
  }

  SolomonLoader::loadHeader(*this, file);
  SolomonLoader::loadNodes(*this, file);
}


