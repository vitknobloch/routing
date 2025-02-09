//
// Created by knoblvit on 9.2.25.
//

#include "routing_instance.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <regex>

#pragma once

class RoutingInstance::TSPlibLoader{
public:
  static void loadHeader(RoutingInstance &instance, std::ifstream &file);
  static std::string parseName(std::string &line);
  static ProblemType parseType(std::string &line);
  static std::string parseComment(std::string &line);
  static EdgeWeightType parseEdgeWeightType(std::string &line);
  static EdgeWeightFormat parseEdgeWeightFormat(std::string &line);
  static DisplayDataType parseDisplayDataType(std::string &line);

  static void loadNodes(RoutingInstance &instance, std::ifstream  &file);
  static void loadNodes_EUC_2D(RoutingInstance &instance, std::ifstream &file);
  static void loadNodes_GEO(RoutingInstance &instance, std::ifstream  &file);
  static void loadNodes_CEIL_2D(RoutingInstance &instance, std::ifstream &file);
  static void loadNodes_FULL_MATRIX(RoutingInstance &instance, std::ifstream &file);

  static void fillNodes(RoutingInstance &instance);
  static void loadDemand(RoutingInstance &instance, std::ifstream &file);
  static void loadDepos(RoutingInstance &instance, std::ifstream &file);

  static inline std::string strip(const std::string& line);
  static inline std::pair<double, double> latitudeLongitude(double x, double y);
};
