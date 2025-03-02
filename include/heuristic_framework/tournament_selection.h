//
// Created by knoblvit on 28.2.25.
//

#pragma once

#include "selection.h"
#include <random>

class TournamentSelection : public Selection{
private:
  uint tournament_size_;
  std::mt19937 gen_;

public:
  explicit TournamentSelection(uint tournament_size);
  std::vector<uint> select(std::shared_ptr<Population> &population, uint select_count) override;
};