#pragma once

#include "heuristic_framework/mutation.h"
#include "VRP-TW/vrptw_structured_individual.h"
#include <random>

class VrptwMutationReinsert : public Mutation{
private:
  std::random_device rand;
  std::mt19937 gen;
  double mutation_rate_;

  VrptwRouteSegment selectSource(const std::shared_ptr<VrptwIndividualStructured> &individual);
  VrptwRouteSegment selectTarget(const std::shared_ptr<VrptwIndividualStructured> &individual, const VrptwRouteSegment &source);

public:
  VrptwMutationReinsert();
  bool isInPlace() override;
  bool mutate(const std::shared_ptr<Individual> &individual) override;
  double getMutationRate() override;
  void setMutationRate(double mutation_rate);
};