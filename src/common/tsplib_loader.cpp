//
// Created by knoblvit on 9.2.25.
//
#include "common/tsplib_loader.h"

inline std::string RoutingInstance::TSPlibLoader::strip(const std::string& line) {
  auto begin_idx = line.find_first_not_of(" \n\r\t\v");
  auto last_idx = line.find_last_not_of(" \n\r\t\v");
  return line.substr(begin_idx, last_idx + 1 - begin_idx);
}

inline std::pair<double, double>
RoutingInstance::TSPlibLoader::latitudeLongitude(double x, double y) {
  double degreesX = round(x);
  double minutesX = x - degreesX;
  double latitude = M_PI * (degreesX + 5.0 * minutesX / 3.0) / 180.0;
  double degreesY = round(y);
  double minutesY = y - degreesY;
  double longitude = M_PI * (degreesY + 5.0 * minutesY / 3.0) / 180.0;
  return {latitude, longitude};
}

std::string RoutingInstance::TSPlibLoader::parseName(std::string &line) {
  return strip(line.substr(line.find(':') + 1));
}

ProblemType RoutingInstance::TSPlibLoader::parseType(std::string &line) {
  std::string type = strip(line.substr(line.find(':') + 1));
  if(type == "TSP")
    return ProblemType::TSP;
  else if(type == "ATSP")
    return ProblemType::ATSP;
  else if(type == "CVRP")
    return ProblemType::CVRP;
  else{
    std::cerr << "Unrecognized problem type: " << type << std::endl;
    exit(100);
  }
}

std::string RoutingInstance::TSPlibLoader::parseComment(std::string &line) {
  return strip(line.substr(line.find(':') + 1));
}

EdgeWeightType
RoutingInstance::TSPlibLoader::parseEdgeWeightType(std::string &line) {
  std::string type = strip(line.substr(line.find(':') + 1));
  if(type == "GEO")
    return EdgeWeightType::GEO;
  else if(type == "EUC_2D")
    return EdgeWeightType::EUC_2D;
  else if(type == "CEIL_2D")
    return EdgeWeightType::CEIL_2D;
  else if(type == "EXPLICIT")
    return EdgeWeightType::EXPLICIT;
  else{
    std::cerr << "Unrecognized edge weight type: " << type << std::endl;
    exit(100);
  }
}

EdgeWeightFormat
RoutingInstance::TSPlibLoader::parseEdgeWeightFormat(std::string &line) {
  std::string format = strip(line.substr(line.find(':') + 1));
  if(format == "FUNCTION")
    return EdgeWeightFormat::FUNCTION;
  else if(format == "FULL_MATRIX")
    return EdgeWeightFormat::FULL_MATRIX;
  else{
    std::cerr << "Unrecognized edge weight format: " << format << std::endl;
    exit(100);
  }
}

DisplayDataType
RoutingInstance::TSPlibLoader::parseDisplayDataType(std::string &line) {
  std::string type = strip(line.substr(line.find(':') + 1));
  if(type == "COORD_DISPLAY")
    return DisplayDataType::COORD_DISPLAY;
  else if(type == "TWOD_DISPLAY")
    return DisplayDataType::TWOD_DISPLAY;
  else if(type == "NO_DISPLAY")
    return DisplayDataType::NO_DISPLAY;
  else{
    std::cerr << "Unrecognized display data type: " << type << std::endl;
    exit(100);
  }
}


void RoutingInstance::TSPlibLoader::loadNodes(RoutingInstance &instance, std::ifstream &file) {
  if(instance.edge_weight_type_ == EUC_2D){
    loadNodes_EUC_2D(instance, file);
    //correctMatrix();
  }
  else if(instance.edge_weight_type_ == CEIL_2D){
    loadNodes_CEIL_2D(instance, file);
    //correctMatrix();
  }
  else if(instance.edge_weight_type_ == GEO){
    loadNodes_GEO(instance, file);
    //correctMatrix();
  }
  else if(instance.edge_weight_type_ == EXPLICIT){
    loadNodes_FULL_MATRIX(instance, file);
  }
  else{
    throw std::domain_error("Not implemented combination of problem type and edge weight type");
  }

  for(int i = 0; i < instance.node_count_; i++)
    instance.matrix_[i * instance.node_count_ + i] = 0;

  file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void RoutingInstance::TSPlibLoader::loadNodes_EUC_2D(RoutingInstance &instance,
                                                     std::ifstream &file) {
  float dump;
  std::unique_ptr<float[]> nodes_x = std::unique_ptr<float[]>(new float[instance.node_count_]);
  std::unique_ptr<float[]> nodes_y = std::unique_ptr<float[]>(new float[instance.node_count_]);
  for(int i = 0; i < instance.node_count_; i++){
    file >> dump;
    if(dump != i + 1.0){
      std::cerr << "Error loading input node_idx doesn't match" << std::endl;
      exit(110);
    }
    file >> nodes_x[i];
    file >> nodes_y[i];
  }

  instance.matrix_ = std::shared_ptr<unsigned int[]>(new unsigned int[instance.node_count_ * instance.node_count_]);

  for(int i = 0; i < instance.node_count_; i++){
    for(int j = 0; j < instance.node_count_; j++){
      auto dist = (unsigned int)round(sqrt(pow(nodes_x[i] - nodes_x[j], 2) + pow(nodes_y[i] - nodes_y[j], 2)));
      instance.matrix_[i * instance.node_count_ + j] = dist;
    }
  }
}

void RoutingInstance::TSPlibLoader::loadNodes_CEIL_2D(RoutingInstance &instance,
                                                      std::ifstream &file) {
  float dump;
  std::unique_ptr<float[]> nodes_x = std::unique_ptr<float[]>(new float[instance.node_count_]);
  std::unique_ptr<float[]> nodes_y = std::unique_ptr<float[]>(new float[instance.node_count_]);
  for(int i = 0; i < instance.node_count_; i++){
    file >> dump;
    if(dump != i + 1.0){
      std::cerr << "Error loading input node_idx doesn't match" << std::endl;
      exit(110);
    }
    file >> nodes_x[i];
    file >> nodes_y[i];
  }

  instance.matrix_ = std::shared_ptr<unsigned int[]>(new unsigned int[instance.node_count_ * instance.node_count_]);

  for(int i = 0; i < instance.node_count_; i++){
    for(int j = 0; j < instance.node_count_; j++){
      auto dist = (unsigned int)ceil(sqrt(pow(nodes_x[i] - nodes_x[j], 2) + pow(nodes_y[i] - nodes_y[j], 2)));
      instance.matrix_[i * instance.node_count_ + j] = dist;
    }
  }
}

void RoutingInstance::TSPlibLoader::loadNodes_GEO(RoutingInstance &instance,
                                                  std::ifstream &file) {
  float dump;
  std::unique_ptr<float[]> nodes_x = std::unique_ptr<float[]>(new float[instance.node_count_]);
  std::unique_ptr<float[]> nodes_y = std::unique_ptr<float[]>(new float[instance.node_count_]);
  for(int i = 0; i < instance.node_count_; i++){
    file >> dump;
    if(dump != i + 1.0){
      std::cerr << "Error loading input node_idx doesn't match" << std::endl;
      exit(110);
    }
    file >> nodes_x[i];
    file >> nodes_y[i];
  }

  instance.matrix_ = std::shared_ptr<unsigned int[]>(new unsigned int[instance.node_count_ * instance.node_count_]);

  for(int i = 0; i < instance.node_count_; i++){
    for(int j = 0; j < instance.node_count_; j++){
      auto lat_long_i = latitudeLongitude(nodes_x[i], nodes_y[i]);
      auto lat_long_j = latitudeLongitude(nodes_x[j], nodes_y[j]);
      double q1 = cos(lat_long_i.second - lat_long_j.second);
      double q2 = cos(lat_long_i.first - lat_long_j.first);
      double q3 = cos(lat_long_i.first + lat_long_j.first);
      auto dist = (unsigned int)round(6378.388 * acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);
      instance.matrix_[i * instance.node_count_ + j] = dist;
    }
  }
}

void RoutingInstance::TSPlibLoader::loadNodes_FULL_MATRIX(
    RoutingInstance &instance, std::ifstream &file) {
  instance.matrix_ = std::shared_ptr<unsigned int[]>(new unsigned int[instance.node_count_ * instance.node_count_]);
  unsigned int input;
  for(int i = 0; i < instance.node_count_; i++){
    for(int j = 0; j < instance.node_count_; j++){
      file >> input;
      instance.matrix_[i * instance.node_count_ + j] = input;
    }
  }
}

void RoutingInstance::TSPlibLoader::fillNodes(RoutingInstance &instance) {
  instance.nodes_ = std::vector<Node>(instance.node_count_, Node());
  for(uint i = 0; i < instance.nodes_.size(); i++){
    instance.nodes_[i].idx = i;
    instance.nodes_[i].service_time = 0;
    instance.nodes_[i].demand = 0;
    instance.nodes_[i].ready_time = 0;
    instance.nodes_[i].due_date = INT32_MAX;
  }
}

void RoutingInstance::TSPlibLoader::loadDemand(RoutingInstance &instance,
                                               std::ifstream &file) {
  for(int i = 0; i < instance.node_count_; i++){
    int dump, demand;
    file >> dump >> demand;
    assert(dump == i + 1);
    instance.nodes_[i].demand = demand;
  }
  file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void RoutingInstance::TSPlibLoader::loadDepos(RoutingInstance &instance,
                                              std::ifstream &file) {
  std::vector<int> depos;
  int depo;
  file >> depo;
  while(depo != -1){
    depos.push_back(depo - 1);
    assert(instance.nodes_[depo - 1].demand == 0); // Depots have to have zero demand
    file >> depo;
  }
  file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  assert(depos.size() == 1); // not implemented for more depots
  assert(depos[0] == 0); // not implemented for other depot than 0

  instance.depots_ = {0};
}

void RoutingInstance::TSPlibLoader::loadHeader(RoutingInstance &instance,
                                               std::ifstream &file) {
  const std::regex name_regex = std::regex("^ *NAME *:");
  const std::regex type_regex = std::regex("^ *TYPE *:");
  const std::regex comment_regex = std::regex("^ *COMMENT *:");
  const std::regex dimension_regex = std::regex("^ *DIMENSION *:");
  const std::regex edge_weight_type_regex = std::regex("^ *EDGE_WEIGHT_TYPE *:");
  const std::regex edge_weight_format_regex = std::regex("^ *EDGE_WEIGHT_FORMAT *:");
  const std::regex display_data_type_regex = std::regex("^ *DISPLAY_DATA_TYPE *:");
  const std::regex node_coord_section_regex = std::regex("^ *NODE_COORD_SECTION *");
  const std::regex edge_weight_section_regex = std::regex("^ *EDGE_WEIGHT_SECTION *");
  const std::regex demand_regex = std::regex("^ *DEMAND_SECTION *");
  const std::regex depot_regex = std::regex("^ *DEPOT_SECTION *");
  const std::regex capacity_regex = std::regex("^ *CAPACITY *:");
  const std::regex eof_regex = std::regex("^ *EOF *");

  std::string line;
  while( std::getline(file, line) )
  {
    if(line.empty())
      continue;

    if(std::regex_search(line, name_regex)){
      instance.instance_name_ = parseName(line);
    }
    else if(std::regex_search(line, type_regex)){
      instance.problem_type_ = parseType(line);
    }
    else if(std::regex_search(line, comment_regex)){
      instance.comment_ = parseComment(line);
    }
    else if(std::regex_search(line, dimension_regex)){
      instance.node_count_ = std::stoi(line.substr(line.find(':') + 1));
      fillNodes(instance);
    }
    else if(std::regex_search(line, edge_weight_type_regex)){
      instance.edge_weight_type_ = parseEdgeWeightType(line);
    }
    else if(std::regex_search(line, edge_weight_format_regex)){
      instance.edge_weight_format_ = parseEdgeWeightFormat(line);
    }
    else if(std::regex_search(line, display_data_type_regex)) {
      instance.display_data_type_ = parseDisplayDataType(line);
    }
    else if(std::regex_search(line, capacity_regex)){
      std::string cap_str = strip(line.substr(line.find(':') + 1));
      instance.vehicle_capacity_ = std::stoi(cap_str);
    }
    else if(std::regex_search(line, node_coord_section_regex)){
      loadNodes(instance, file);
    }
    else if(std::regex_search(line, edge_weight_section_regex)){
      loadNodes(instance, file);
    }
    else if(std::regex_search(line, demand_regex)){
      loadDemand(instance, file);
    }
    else if(std::regex_search(line, depot_regex)){
      loadDepos(instance, file);
    }
    else if(std::regex_search(line, eof_regex)){
      break;
    }
    else{
      std::cerr << "Unrecognized input line: " << line << std::endl;
      exit(100);
    }
  }
}