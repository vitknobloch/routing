//
// Created by knoblvit on 26.4.25.
//

#pragma once

#include "heuristic_framework/crossover.h"
#include "VRP-TW/vrptw_structured_individual.h"
#include <random>

class VrptwPmxCrossoverStructured : public Crossover{
private:
  double crossover_rate_;
  std::mt19937 gen_;

  std::shared_ptr<VrptwIndividualStructured> crossoverTwo(
      const std::shared_ptr<VrptwIndividualStructured> &parent1,
      const std::shared_ptr<VrptwIndividualStructured> &parent2,
      uint exchange_start, uint exchange_end
  );

public:
  VrptwPmxCrossoverStructured();
  void setCrossoverRate(double crossover_rate);
  std::shared_ptr<Population> crossover(const std::shared_ptr<Population> &population, const std::vector<uint> &parents) override;
  double getCrossoverRate() override;
};