//
// Created by knoblvit on 24.2.25.
//

#pragma once
#include "heuristic_framework/mutation.h"
#include <random>

class TspMutation2opt : public Mutation{
private:
  std::random_device rand;
  std::mt19937 gen;
  const uint * const matrix_;
  double mutation_rate_;

public:
  TspMutation2opt(const uint * const matrix);
  bool isInPlace() override;
  bool mutate(const std::shared_ptr<Individual> &individual) override;
  double getMutationRate() override;
  void setMutationRate(double mutation_rate);
};
