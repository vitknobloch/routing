#pragma once

#include "heuristic_framework/individual.h"
#include "common/routing_instance.h"
#include "common/solution.h"

struct TspIndividualSegment{
  uint start_idx;
  uint end_idx;
};

class TspIndividualStructured: public Individual{
private:
  const RoutingInstance* const instance_;
  std::vector<uint> data_;
  bool is_evaluated_;
  uint total_time_;

public:
  explicit TspIndividualStructured(const RoutingInstance* const instance);
  TspIndividualStructured(const RoutingInstance* const instance, const std::shared_ptr<Solution> &solution);
  TspIndividualStructured(const TspIndividualStructured &cpy);
  TspIndividualStructured(const TspIndividualStructured &cpy, const std::vector<uint> &flat_data);
  std::shared_ptr<Solution> convertSolution();
  void initialize() override;
  void smartInitialize() override;
  void resetEvaluated() override;
  bool betterThan(const std::shared_ptr<Individual> &other) override;
  double getFitness() override;
  void calculateConstraints() override;
  void calculateFitness() override;
  const std::vector<double> & getConstraintViolations() override;
  double getTotalConstraintViolation() override;
  bool isEvaluated() override;
  std::shared_ptr<Individual> deepcopy() override;
  void evaluate() override;
  std::vector<uint> flatten();
  ~TspIndividualStructured() override = default;

  const std::vector<uint> &getData();

  void perform2optMove(const TspIndividualSegment &segment);

  bool test2optMove(const TspIndividualSegment &segment);

  double get2optMoveCost(const TspIndividualSegment &segment);

  void performDoubleBridgeMove(const TspIndividualSegment &segment1, const TspIndividualSegment &segment2);
};