#include "TSP/tsp_SA_step.h"
#include <cassert>
#include <iostream>

TspSAStep::TspSAStep() : rand_(), gen_(rand_()), dist_(0.0, 1.0) {
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    try_count[i] = 1;
    success_count[i] = 1;
  }
}

TspSAStep::neighborhood_options TspSAStep::selectNeighborhoodOption() {
  double max_val = 0;
  double vals[neighborhood_options::SIZE];
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    max_val += success_count[i] / (1 + std::log(try_count[i]));
    vals[i] = max_val;
  }

  if(max_val == 0)
    return neighborhood_options::SIZE;

  for(double & val : vals){
    val /= max_val;
  }

  double choice = dist_(gen_);
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    if(choice <= vals[i])
      return static_cast<neighborhood_options>(i);
  }

  assert(false);
  return neighborhood_options::SIZE; // This shouldn't happen
}

void TspSAStep::success(TspSAStep::neighborhood_options neighborhood) {
  try_count[neighborhood] += 1;
  success_count[neighborhood] += 1;
}

void TspSAStep::failure(TspSAStep::neighborhood_options neighborhood) {
  try_count[neighborhood] += 1;
}

StepResult TspSAStep::step(const std::shared_ptr<Individual> &individual,
                           const std::shared_ptr<SASchedule> &schedule,
                           const std::shared_ptr<Individual> &best_individual) {
  const auto &individual_ = std::static_pointer_cast<TspIndividualStructured>(individual);
  const auto &best_individual_ = std::static_pointer_cast<TspIndividualStructured>(best_individual);
  neighborhood_options selected = selectNeighborhoodOption();
  if(selected == neighborhood_options::SIZE)
    return SABasicScheduleMemory::UNACCEPTED;

  StepResult result = StepResult::UNACCEPTED;
  if(selected == neighborhood_options::TWO_OPT)
    result = perform2opt(individual_, schedule, best_individual_);
  else if(selected == neighborhood_options::SWAP)
    result = performSwap(individual_, schedule, best_individual_);
  else if(selected == neighborhood_options::RELOCATE)
    result = performRelocate(individual_, schedule, best_individual_);

  if(result == StepResult::IMPROVED){
    success(selected);
    return result;
  }
  else{
    failure(selected);
    return result;
  }
}

void TspSAStep::reset(const std::shared_ptr<Individual> &individual) {
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    try_count[i] = 1;
    success_count[i] = 1;
  }
}

StepResult TspSAStep::perform2opt(
    const std::shared_ptr<TspIndividualStructured> &individual,
    const std::shared_ptr<SASchedule> &schedule,
    const std::shared_ptr<TspIndividualStructured> &best_individual) {
  std::uniform_int_distribution<uint> start_dist(0, individual->getData().size()-1);
  uint first = start_dist(gen_);
  uint second = start_dist(gen_);
  while(first == second)
    second = start_dist(gen_);

  TspIndividualSegment segment{std::min(first, second), std::max(first, second)};
  const auto diff = individual->get2optMoveCost(segment);
  const auto total_diff = diff + individual->getFitnessDiff(*best_individual);

  if(diff.fitness <= 0){
    //std::cerr << "+ 2opt " << diff.fitness << " " << individual->getFitness() << std::endl;
    individual->perform2optMove(segment);
    return stepResult::IMPROVED;
  }
  else if(schedule->shouldAcceptSolution(total_diff)){
    individual->perform2optMove(segment);
    return stepResult::ACCEPTED;
  }
  return StepResult::UNACCEPTED;
}

StepResult TspSAStep::performSwap(
    const std::shared_ptr<TspIndividualStructured> &individual,
    const std::shared_ptr<SASchedule> &schedule,
    const std::shared_ptr<TspIndividualStructured> &best_individual) {
  std::uniform_int_distribution<uint> start_dist(0, individual->getData().size()-1);
  uint first = start_dist(gen_);
  uint second = start_dist(gen_);
  while(std::abs((int)first - (int)second) < 2)
    second = start_dist(gen_);

  const auto diff = individual->getSwapMoveCost(first, second);
  const auto total_diff = diff + individual->getFitnessDiff(*best_individual);

  if(diff.fitness <= 0){
    //std::cerr << "+ swap " << diff.fitness << " " << individual->getFitness() << std::endl;
    individual->performSwapMove(first, second);
    return stepResult::IMPROVED;
  }
  else if(schedule->shouldAcceptSolution(total_diff)){
    individual->performSwapMove(first, second);
    return stepResult::ACCEPTED;
  }
  return StepResult::UNACCEPTED;
}

StepResult TspSAStep::performRelocate(
    const std::shared_ptr<TspIndividualStructured> &individual,
    const std::shared_ptr<SASchedule> &schedule,
    const std::shared_ptr<TspIndividualStructured> &best_individual) {
  std::uniform_int_distribution<uint> start_dist(0, individual->getData().size()-1);
  uint first = start_dist(gen_);
  uint second = start_dist(gen_);
  while(first == second)
    second = start_dist(gen_);

  const auto diff = individual->getRelocateMoveCost(first, second);
  const auto total_diff = diff + individual->getFitnessDiff(*best_individual);

  if(diff.fitness <= 0){
    //std::cerr << "+ relo " << diff.fitness << " " << individual->getFitness() << std::endl;
    individual->performRelocateMove(first, second);
    return stepResult::IMPROVED;
  }
  else if(schedule->shouldAcceptSolution(total_diff)){
    individual->performRelocateMove(first, second);
    return stepResult::ACCEPTED;
  }
  return StepResult::UNACCEPTED;
}
