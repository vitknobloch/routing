#include "heuristic_framework/population_stochastic_ranking.h"
#include <algorithm>
#include <functional>

PopulationStochasticRanking::PopulationStochasticRanking(double fitness_comp_prob) : Population(), idx_to_rank_() {
  std::random_device rand;
  gen_ = std::mt19937(rand());
  dist_ = std::bernoulli_distribution(fitness_comp_prob);
}

void PopulationStochasticRanking::rank() {
  ranks_.resize(population_.size());
  idx_to_rank_.resize(population_.size());
  for(uint i = 0; i < size(); i++){
    ranks_[i] = i;
  }

  std::shuffle(ranks_.begin(), ranks_.end(), gen_);

  uint comp_count = ((size() + 1) * size()) / 2;
  std::vector<bool> comparisons(comp_count);
  auto generator = std::bind(dist_, gen_);
  std::generate_n(comparisons.begin(), comparisons.size(), generator);
  uint k = 0;
  for(uint i = 0; i < size(); i++){
    bool swap_done = false;
    for(uint j = 0; j < size() - 1 - i; j++){
      if(stochasticallyWorseThan(population_[ranks_[j]], population_[ranks_[j+1]], comparisons[k])){
        swap_done = true;
        std::swap(ranks_[j], ranks_[j+1]);
      }
      k++;
    }
    if(!swap_done)
      break;
  }

  for(uint i = 0; i < size(); i++){
    idx_to_rank_[ranks_[i]] = i;
  }
}

bool PopulationStochasticRanking::isBetter(int idx, int idx_other) {
  if(!ranked_)
    rank();

  return idx_to_rank_[idx] < idx_to_rank_[idx_other];
}

void PopulationStochasticRanking::merge(
    const std::shared_ptr<Population> &other) {
  Population::merge(other);
}

std::shared_ptr<Population> PopulationStochasticRanking::getEmpty() {
  return std::make_shared<PopulationStochasticRanking>(dist_.p());
}

void PopulationStochasticRanking::setFitnessCompareProbability(double fitness_comp_prob) {
  dist_ = std::bernoulli_distribution(fitness_comp_prob);
}

inline bool PopulationStochasticRanking::stochasticallyWorseThan(
    const std::shared_ptr<Individual> &individual,
    const std::shared_ptr<Individual> &other, const bool &compare_fitness) {

  if(individual->getTotalConstraintViolation() == other->getTotalConstraintViolation() || compare_fitness){
    if(individual->getFitness() > other->getFitness()){
      return true;
    }
  }else{
    if(individual->getTotalConstraintViolation() > other->getTotalConstraintViolation()){
      return true;
    }
  }
  return false;
}

void PopulationStochasticRanking::shrink(uint new_size) {
  if(!ranked_)
    rank();
  if(new_size > size())
    new_size = size();

  for(uint i = 0; i < new_size; i++){
    uint idx = ranks_[i];
    std::swap(population_[idx], population_[i]);
    ranks_[idx_to_rank_[i]] = idx;
  }

  for(uint i = 0; i < new_size; i++){
    ranks_[i] = i;
    idx_to_rank_[i] = i;
  }

  population_.resize(new_size);
  ranks_.resize(new_size);
  idx_to_rank_.resize(new_size);
}
