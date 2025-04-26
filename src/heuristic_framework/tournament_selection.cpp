#include "heuristic_framework/tournament_selection.h"
#include <algorithm>
#include <cassert>
#include <functional>

TournamentSelection::TournamentSelection(uint tournament_size) {
  tournament_size_ = tournament_size;
  std::random_device rand;
  gen_ = std::mt19937(rand());
}

std::vector<uint> TournamentSelection::
    select(std::shared_ptr<Population> &population, uint select_count) {
  auto selected = std::vector<uint>();
  selected.reserve(select_count);

  const size_t random_trials = select_count * tournament_size_;
  std::vector<uint> rand_vector(random_trials);
  std::uniform_int_distribution<uint> distribution(0, population->size() - 1);
  auto generator = std::bind(distribution, gen_);
  std::generate_n(rand_vector.begin(), random_trials, generator);

  for(uint i = 0; i < select_count; i++){
    const uint first_idx = i * tournament_size_;
    uint best_in_tournament = rand_vector[first_idx];
    for(uint j = 1; j < tournament_size_; j++)
      if(population->isBetter((int)rand_vector[first_idx + j], (int)best_in_tournament))
        best_in_tournament = rand_vector[first_idx + j];
    selected.push_back(best_in_tournament);
  }

  assert(selected.size() == select_count);
  return selected;
}
