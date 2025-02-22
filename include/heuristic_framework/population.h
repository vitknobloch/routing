//
// Created by knoblvit on 20.2.25.
//

#pragma once

#include <memory>
#include <vector>
#include "individual.h"

class Population{
protected:
  std::shared_ptr<Individual> best_individual_;
  std::vector<std::shared_ptr<Individual>> population_;

public:
  const std::shared_ptr<Individual> &getBestIndividual() const;
  const std::vector<std::shared_ptr<Individual>> &getPopulation() const;
  const std::shared_ptr<Individual> &getIndividual(int idx);
  size_t size() const;
  void addIndividual(const std::shared_ptr<Individual> &individual);
  void replaceIndividual(const std::shared_ptr<Individual> &individual);
  void merge(const std::shared_ptr<Population> &other);
  void evaluate();
};