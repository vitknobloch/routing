#pragma once
#include "TSP/tsp_individual.h"
#include "heuristic_framework/crossover.h"
#include <random>

class TspPmxCrossover : public Crossover{
private:
  double crossover_rate_;
  std::mt19937 gen_;

  std::shared_ptr<TspIndividual> crossoverTwo(
      const std::shared_ptr<TspIndividual> parent1,
      const std::shared_ptr<TspIndividual> parent2,
      uint exchange_start, uint exchange_end);
public:
  TspPmxCrossover();
  void setCrossoverRate(double crossover_rate);
  std::shared_ptr<Population> crossover(const std::shared_ptr<Population> &population, const std::vector<uint> &parents) override;
  double getCrossoverRate() override;
};