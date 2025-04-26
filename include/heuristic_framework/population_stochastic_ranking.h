#pragma once
#include "heuristic_framework/population.h"
#include <random>

class PopulationStochasticRanking : public Population {
private:
  std::vector<uint> idx_to_rank_;
  std::mt19937 gen_;
  std::bernoulli_distribution dist_;

  inline bool stochasticallyWorseThan(const std::shared_ptr<Individual> &individual, const std::shared_ptr<Individual> &other, const bool &compare_fitness);

public:
  PopulationStochasticRanking(double fitness_comp_prob);
  void setFitnessCompareProbability(double fitness_comp_prob);
  void shrink(uint new_size);

  void rank() override;
  bool isBetter(int idx, int idx_other) override;
  void merge(const std::shared_ptr<Population> &other) override;
  std::shared_ptr<Population> getEmpty() override;

};
