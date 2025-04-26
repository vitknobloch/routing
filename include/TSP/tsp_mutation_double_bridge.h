#pragma once

#include "heuristic_framework/mutation.h"
#include "TSP/tsp_individual_structured.h"
#include <random>

class TspMutationDoubleBridge : public Mutation{
private:
  std::random_device rand;
  std::mt19937 gen;
  double mutation_rate_;

public:
  TspMutationDoubleBridge();
  bool isInPlace() override;
  bool mutate(const std::shared_ptr<Individual> &individual) override;
  double getMutationRate() override;
  void setMutationRate(double mutation_rate);
};
