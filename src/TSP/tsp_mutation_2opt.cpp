#include "TSP/tsp_mutation_2opt.h"
#include "TSP/tsp_individual.h"
#include <cassert>
#include <iostream>

bool TspMutation2opt::isInPlace() {
  return true;
}

bool TspMutation2opt::mutate(const std::shared_ptr<Individual> &individual) {
  auto individual_ = std::static_pointer_cast<TspIndividual>(individual);
  auto &data = individual_->data();
  std::uniform_int_distribution<uint> dist(0, data.size() - 1);

  const uint start = dist(gen);
  uint length = dist(gen);
  if(length == data.size() - 1)
    return true;
  const uint end = (start + length) % data.size();
  const uint start_node = data[start];
  const uint end_node = data[end];
  const uint prev_node = data[(start + data.size() - 1) % data.size()];
  const uint next_node = data[(end + 1) % data.size()];
  const uint matrix_size = data.size();

  const uint prev_cost = matrix_[prev_node * matrix_size + start_node] + matrix_[end_node * matrix_size + next_node];
  const uint new_cost = matrix_[prev_node * matrix_size + end_node] + matrix_[start_node * matrix_size + next_node];
  if(new_cost >= prev_cost){
    return true;
  }

  for(uint i = 0; i <= length / 2; i++){
    uint temp = data[(start + i) % matrix_size];
    data[(start + i) % matrix_size] = data[(end + matrix_size - i) % matrix_size];
    data[(end + matrix_size - i) % matrix_size] = temp;
  }

  double new_fitness = individual_->getFitness() - (double)prev_cost + (double)new_cost;
  individual_->setFitness(new_fitness);

  return true;
}

TspMutation2opt::TspMutation2opt(const uint * const matrix) : matrix_(matrix) {
  gen = std::mt19937(rand());
  mutation_rate_ = 1.0;
}
void TspMutation2opt::setMutationRate(double mutation_rate) {
  assert(mutation_rate >= 0.0 && mutation_rate <= 1.0);
  mutation_rate_ = mutation_rate;
}
double TspMutation2opt::getMutationRate() { return mutation_rate_;}
