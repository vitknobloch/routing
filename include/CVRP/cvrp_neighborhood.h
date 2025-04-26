#pragma once
#include "heuristic_framework/neighborhood.h"
#include "CVRP/cvrp_structured_individual.h"
#include <random>

class CvrpNeighborhood : public Neighborhood {
private:
  enum neighborhood_options{
    TWO_OPT,
    EXCHANGE,
    RELOCATE,
    CROSS,
    SIZE
  };
  bool exhausted_[neighborhood_options::SIZE]{};
  double try_count[neighborhood_options::SIZE]{};
  double success_count[neighborhood_options::SIZE]{};
  std::random_device rand_;
  std::mt19937 gen_;
  std::uniform_real_distribution<double> dist_;

  bool perform2opt(const std::shared_ptr<CvrpIndividualStructured> &individual);
  bool performExchange(const std::shared_ptr<CvrpIndividualStructured> &individual);
  bool performRelocate(const std::shared_ptr<CvrpIndividualStructured> &individual);
  bool performCross(const std::shared_ptr<CvrpIndividualStructured> &individual);

  neighborhood_options selectNeighborhoodOption();
  void success(neighborhood_options neighborhood);
  void failure(neighborhood_options neighborhood);

public:
  CvrpNeighborhood();
  SearchResult search(const std::shared_ptr<Individual> &individual) override;
  void reset(const std::shared_ptr<Individual> &individual) override;
};
