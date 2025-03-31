//
// Created by knoblvit on 9.3.25.
//

#pragma once

#include "heuristic_framework/individual.h"
#include "common/routing_instance.h"
#include "common/solution.h"

using uint = unsigned int;

class VrptwIndividual : public Individual{
private:
  const RoutingInstance* const instance_;
  const uint * const matrix_;
  std::vector<uint> data_;
  bool is_evaluated_;
  double fitness_;
  double total_constraint_violation_;
  std::vector<double> constraint_violations_;

public:
  explicit VrptwIndividual(const RoutingInstance* const instance);
  explicit VrptwIndividual(const RoutingInstance* const instance, const std::shared_ptr<Solution> &solution);
  VrptwIndividual(const VrptwIndividual &other);
  void initialize() override;
  void smartInitialize() override;
  std::vector<uint> &data();
  void resetEvaluated() override;
  bool assertData();
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
  std::shared_ptr<Solution> convertSolution();
  ~VrptwIndividual() override = default;
};
