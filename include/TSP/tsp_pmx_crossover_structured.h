//
// Created by knoblvit on 22.4.25.
//

#pragma once

#include "heuristic_framework/crossover.h"
#include "TSP/tsp_individual_structured.h"
#include <random>

class TspPmxCrossoverStructured : public Crossover{
private:
  double crossover_rate_;
  std::mt19937 gen_;

  std::shared_ptr<TspIndividualStructured> crossoverTwo(
      const std::shared_ptr<TspIndividualStructured> &parent1,
      const std::shared_ptr<TspIndividualStructured> &parent2,
      uint exchange_start, uint exchange_end
      );

public:
  TspPmxCrossoverStructured();
  void setCrossoverRate(double crossover_rate);
  std::shared_ptr<Population> crossover(const std::shared_ptr<Population> &population, const std::vector<uint> &parents) override;
  double getCrossoverRate() override;
};