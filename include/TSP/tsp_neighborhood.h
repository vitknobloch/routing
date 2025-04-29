#pragma once
#include "heuristic_framework/neighborhood.h"
#include "TSP/tsp_individual_structured.h"
#include <random>

class TspNeighborhood : public Neighborhood{
private:
  enum neighborhood_options{
    TWO_OPT,
    SWAP,
    RELOCATE,
    SIZE
  };
  bool exhausted_[neighborhood_options::SIZE]{};
  double try_count[neighborhood_options::SIZE]{};
  double success_count[neighborhood_options::SIZE]{};
  std::random_device rand_;
  std::mt19937 gen_;
  std::uniform_real_distribution<double> dist_;

  bool perform2opt(const std::shared_ptr<TspIndividualStructured> &individual);
  bool performSwap(const std::shared_ptr<TspIndividualStructured> &individual);
  bool performRelocate(const std::shared_ptr<TspIndividualStructured> &individual);

  neighborhood_options selectNeighborhoodOption();
  void success(neighborhood_options neighborhood);
  void failure(neighborhood_options neighborhood);

public:
  TspNeighborhood();
  SearchResult search(const std::shared_ptr<Individual> &individual) override;
  void reset(const std::shared_ptr<Individual> &individual) override;
};