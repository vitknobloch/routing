//
// Created by knoblvit on 9.2.25.
//

#pragma once
#include <vector>
#include <string>
#include <memory>

enum ProblemType{
  TSP,
  ATSP,
  CVRP,
  VRPTW
};

enum EdgeWeightType{
  EUC_2D,
  CEIL_2D,
  GEO,
  EXPLICIT
};

enum EdgeWeightFormat{
  FUNCTION,
  FULL_MATRIX
};

enum DisplayDataType{
  COORD_DISPLAY,
  TWOD_DISPLAY,
  NO_DISPLAY
};

struct Node{
  int idx;
  int demand;
  int ready_time;
  int due_date;
  int service_time;

  Node(int idx, int demand, int ready_time, int due_date, int service_time){
    this->idx = idx;
    this->demand = demand;
    this->ready_time = ready_time;
    this->due_date = due_date;
    this->service_time = service_time;
  }

  Node(){
    idx = 0;
    demand = 0;
    ready_time = 0;
    due_date = 0;
    service_time = 0;
  }
};

class RoutingInstance {
private:
  std::vector<Node> nodes_;
  std::vector<uint> depots_;
  std::string instance_name_;
  std::string comment_;
  ProblemType problem_type_;
  EdgeWeightType edge_weight_type_;
  EdgeWeightFormat edge_weight_format_;
  DisplayDataType display_data_type_;
  int vehicle_capacity_;
  int vehicle_count_;
  int node_count_;
  std::shared_ptr<unsigned int[]> matrix_;

  class TSPlibLoader;
  class SolomonLoader;


public:
  RoutingInstance();

  void loadTSPlibInstance(const char *filename);
  void loadSolomonInstance(const char *filename);

  [[nodiscard]] inline const std::vector<Node> &getNodes() const { return nodes_;}
  [[nodiscard]] inline const int &getNodesCount() const { return node_count_;}
  [[nodiscard]] inline std::shared_ptr<unsigned int []> getMatrix() const {return matrix_;}
  [[nodiscard]] inline const std::string &getInstanceName() const {return instance_name_;}
  [[nodiscard]] inline const int &getVehicleCapacity() const {return vehicle_capacity_;}
  [[nodiscard]] inline const int &getVehicleCount() const{ return vehicle_count_;}
  [[nodiscard]] uint getDistance(uint from, uint to) const;

};