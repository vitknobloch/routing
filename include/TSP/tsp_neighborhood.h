//
// Created by knoblvit on 22.4.25.
//

#pragma once
#include "heuristic_framework/neighborhood.h"
#include "TSP/tsp_individual_structured.h"
#include <random>

class TspNeighborhood : public Neighborhood{

public:
  TspNeighborhood();
  SearchResult search(const std::shared_ptr<Individual> &individual) override;
  void reset(const std::shared_ptr<Individual> &individual) override;
};