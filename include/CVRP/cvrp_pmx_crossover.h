//
// Created by knoblvit on 3.3.25.
//

#pragma once
#include "CVRP/cvrp_individual.h"
#include "heuristic_framework/crossover.h"
#include <random>

class CvrpPmxCrossover : public Crossover{
private:
  double crossover_rate_;
  std::mt19937 gen_;

  std::shared_ptr<CvrpIndividual> crossoverTwo(
      const std::shared_ptr<CvrpIndividual> parent1,
      const std::shared_ptr<CvrpIndividual> parent2,
      uint exchange_start, uint exchange_end);

public:
  CvrpPmxCrossover();
  void setCrossoverRate(double crossover_rate);
  std::shared_ptr<Population> crossover(const std::shared_ptr<Population> &population, const std::vector<uint> &parents) override;
  double getCrossoverRate() override;
};