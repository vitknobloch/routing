//
// Created by knoblvit on 24.2.25.
//

#pragma once

#include "heuristic_framework/individual.h"
#include "common/routing_instance.h"
#include "common/solution.h"

using uint = unsigned int;

class CvrpIndividual: public Individual{
private:
  const RoutingInstance* const instance_;
  const uint * const matrix_;
  std::vector<uint> data_;
  bool is_evaluated_;
  double fitness_;
  double capacity_constraint_violation_;

public:
  explicit CvrpIndividual(const RoutingInstance* const instance);
  explicit CvrpIndividual(const RoutingInstance* const instance, const std::shared_ptr<Solution> &solution);
  CvrpIndividual(const CvrpIndividual &other);
  void initialize() override;
  std::vector<uint> &data();
  void resetEvaluated() override;
  bool assertData();
  void setFitness(double fitness);
  bool betterThan(const std::shared_ptr<Individual> &other) override;
  void calculateFitness() override;
  void evaluate() override;
  void smartInitialize() override;
  std::shared_ptr<Individual> deepcopy() override;
  double getFitness() override;
  bool isEvaluated() override;
  void calculateConstraints() override;
  const std::vector<double> &getConstraintViolations() override;
  double getTotalConstraintViolation() override;
  std::shared_ptr<Solution> convertSolution();
  ~CvrpIndividual() override = default;
};
