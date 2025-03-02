//
// Created by knoblvit on 28.2.25.
//
#include "heuristic_framework/truncation_replacement.h"

std::shared_ptr<Population> TruncationReplacement::replacementFunction(
    const std::shared_ptr<Population> &old_pop,
    const std::shared_ptr<Population> &new_pop,
    uint final_size) {
  if(!old_pop->isRanked())
    old_pop->rank();
  if(!new_pop->isRanked())
    new_pop->rank();

  if(final_size == 0)
    final_size = old_pop->size();

  uint old_rank = 0;
  uint new_rank = 0;
  auto pop = old_pop->getEmpty();
  for(uint i = 0; i < final_size; i++){
    if(new_rank >= new_pop->size() || old_pop->getIndividualByRank(old_rank)->betterThan(new_pop->getIndividualByRank(new_rank))){
      pop->addIndividual(old_pop->getIndividualByRank(old_rank));
      old_rank++;
    }else{
      pop->addIndividual(new_pop->getIndividualByRank(new_rank));
    }
    if(new_rank >= new_pop->size() && old_rank >= old_pop->size())
      break;
  }
  return pop;
}
