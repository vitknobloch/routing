//
// Created by knoblvit on 28.2.25.
//
#include "heuristic_framework/population.h"
#include <cassert>
#include <algorithm>

[[maybe_unused]] const std::shared_ptr<Individual> &Population::getBestIndividual() const {
  return best_individual_;
}
const std::vector<std::shared_ptr<Individual>> &
Population::getPopulation() const {
  return population_;
}
const std::shared_ptr<Individual> &Population::getIndividual(int idx) {
  assert(idx >= 0 && idx < population_.size());
  return population_[idx];
}
size_t Population::size() const { return population_.size(); }

void Population::addIndividual(const std::shared_ptr<Individual> &individual) {
 if(individual->betterThan(best_individual_)){
   best_individual_ = individual;
 }
 population_.push_back(individual);
 ranked_ = false;
}
void Population::replaceIndividual(
    int idx, const std::shared_ptr<Individual> &individual) {
  assert(idx >= 0 && idx < population_.size());
  if(individual->betterThan(best_individual_)){
    best_individual_ = individual;
  }
  population_[idx] = individual;
  ranked_ = false;
}

void Population::merge(const std::shared_ptr<Population> &other) {
  if(other->best_individual_->betterThan(best_individual_)){
    best_individual_ = other->best_individual_;
  }

  population_.insert(population_.end(), other->population_.begin(), other->population_.end());
  ranked_ = false;
}

void Population::evaluate() {
  for(auto &individual: population_){
    individual->evaluate();
    if(individual->betterThan(best_individual_))
      best_individual_ = individual;
  }
}

Population::Population() : best_individual_(nullptr), population_(), ranks_(), ranked_(false) {

}

bool Population::isBetter(int idx, int idx_other) {
  assert(idx >= 0 && idx < population_.size());
  assert(idx_other >= 0 && idx_other < population_.size());
  return population_[idx]->betterThan(population_[idx_other]);
}

bool Population::isRanked() { return ranked_; }

void Population::rank() {
  ranks_.resize(population_.size());
  for(uint i = 0; i < size(); i++){
    ranks_[i] = i;
  }

  std::sort(ranks_.begin(), ranks_.end(), [this](const uint &r1, const uint &r2) -> bool { return isBetter(r1, r2);});
  ranked_ = true;
}
std::shared_ptr<Population> Population::getEmpty() {
  return std::make_shared<Population>();
}
const std::shared_ptr<Individual> &Population::getIndividualByRank(int rank) {
  assert(ranked_);
  return population_[ranks_[rank]];
}
