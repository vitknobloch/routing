//
// Created by knoblvit on 22.2.25.
//

#pragma once

#include "heuristic_framework/individual.h"
#include "common/routing_instance.h"

using uint = unsigned int;

class TspIndividual: public Individual{
private:
  const RoutingInstance* const instance_;
  const uint * const matrix_;
  std::vector<uint> data_;
  bool is_evaluated_;
  double fitness_;



public:
  explicit TspIndividual(const RoutingInstance* const instance);
  TspIndividual(const TspIndividual &other);
  void initialize() override;
  void initializeNearestNeighbor();
  std::vector<uint> &data();
  void resetEvaluated();
  void setFitness(double fitness);
  bool betterThan(const std::shared_ptr<Individual> &other) override;
  void calculateFitness() override;
  void evaluate() override;
  std::shared_ptr<Individual> deepcopy() override;
  double getFitness() override;
  bool isEvaluated() override;
  void calculateConstraints() override;
  const std::vector<double> &getConstraintViolations() override;
  double getTotalConstraintViolation() override;
  ~TspIndividual() override = default;
};
