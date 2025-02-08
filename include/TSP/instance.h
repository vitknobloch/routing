//
// Created by knoblvit on 8.2.25.
//

#pragma once

#include <memory>
#include <string>

enum ProblemType{
  TSP,
  ATSP
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

class TspLibInstance {
private:
  std::string name_;
  std::string comment_;
  ProblemType problem_type_;
  EdgeWeightType edge_weight_type_;
  EdgeWeightFormat edge_weight_format_;
  DisplayDataType display_data_type_;
  int dimension_;
  std::shared_ptr<unsigned int[]> matrix_;
  std::shared_ptr<unsigned int[]> neighbours_sorted_;

private:
  void loadHeader(std::ifstream &file);
  std::string parseName(std::string &line);
  ProblemType parseType(std::string &line);
  std::string parseComment(std::string &line);
  EdgeWeightType parseEdgeWeightType(std::string &line);
  EdgeWeightFormat parseEdgeWeightFormat(std::string &line);
  DisplayDataType parseDisplayDataType(std::string &line);

  void loadNodes(std::ifstream  &file);
  void loadNodes_EUC_2D(std::ifstream &file);
  void loadNodes_GEO(std::ifstream  &file);
  void loadNodes_CEIL_2D(std::ifstream &file);
  void loadNodes_FULL_MATRIX(std::ifstream &file);

  void calculateNearestNeighbours();


  inline std::string strip(std::string line);
  inline std::pair<double, double> latitudeLongitude(double x, double y);

public:
  explicit TspLibInstance(std::string filename);
  unsigned int getDistance(const unsigned int &i, const unsigned int &j) const;
  int getDimension() const;
  std::string getName() {return name_;};
  std::string getComment() {return comment_;};
  ProblemType getProblemType() {return problem_type_;}
  std::shared_ptr<unsigned int[]> getMatrix(){return matrix_;};

  friend std::ostream &operator<<(std::ostream &o, const TspLibInstance &instance);
};
