//
// Created by knoblvit on 25.4.25.
//

#pragma once

#include "heuristic_framework/mutation.h"
#include "CVRP/cvrp_structured_individual.h"
#include <random>

class CvrpMutationReinsert : public Mutation{
private:
  std::random_device rand;
  std::mt19937 gen;
  double mutation_rate_;

  CvrpRouteSegment selectSource(const std::shared_ptr<CvrpIndividualStructured> &individual);
  CvrpRouteSegment selectTarget(const std::shared_ptr<CvrpIndividualStructured> &individual, const CvrpRouteSegment &source);

public:
  CvrpMutationReinsert();
  bool isInPlace() override;
  bool mutate(const std::shared_ptr<Individual> &individual) override;
  double getMutationRate() override;
  void setMutationRate(double mutation_rate);
};