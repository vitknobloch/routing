//
// Created by knoblvit on 25.4.25.
//

#pragma once

#include "heuristic_framework/crossover.h"
#include "CVRP/cvrp_structured_individual.h"
#include <random>

class CvrpPmxCrossoverStructured : public Crossover{
private:
  double crossover_rate_;
  std::mt19937 gen_;

  std::shared_ptr<CvrpIndividualStructured> crossoverTwo(
      const std::shared_ptr<CvrpIndividualStructured> &parent1,
      const std::shared_ptr<CvrpIndividualStructured> &parent2,
      uint exchange_start, uint exchange_end
      );

public:
  CvrpPmxCrossoverStructured();
  void setCrossoverRate(double crossover_rate);
  std::shared_ptr<Population> crossover(const std::shared_ptr<Population> &population, const std::vector<uint> &parents) override;
  double getCrossoverRate() override;
};