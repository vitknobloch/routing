//
// Created by knoblvit on 20.2.25.
//

#pragma once

#include "individual.h"
#include <memory>
#include <vector>

using uint = unsigned int;

class Population{
protected:
  std::shared_ptr<Individual> best_individual_;
  std::vector<std::shared_ptr<Individual>> population_;
  std::vector<uint> ranks_;
  bool ranked_;

public:
  Population();
  [[nodiscard]] const std::shared_ptr<Individual> &getBestIndividual() const;
  [[nodiscard]] const std::vector<std::shared_ptr<Individual>> &getPopulation() const;
  const std::shared_ptr<Individual> &getIndividual(int idx);
  [[nodiscard]] size_t size() const;
  void addIndividual(const std::shared_ptr<Individual> &individual);
  void replaceIndividual(int idx, const std::shared_ptr<Individual> &individual);
  virtual void merge(const std::shared_ptr<Population> &other);
  void evaluate();
  virtual bool isBetter(int idx, int idx_other);
  virtual void rank();
  bool isRanked();
  const std::shared_ptr<Individual> &getIndividualByRank(int rank);

  /** Returns empty population of the same type*/
  virtual std::shared_ptr<Population> getEmpty();
};