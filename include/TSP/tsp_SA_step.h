#pragma once

#include "heuristic_framework/simulated_annealing_step.h"
#include "TSP/tsp_individual_structured.h"
#include <random>

using stepResult = SABasicScheduleMemory::StepResult;

class TspSAStep : public SAStep{
private:
  enum neighborhood_options{
    TWO_OPT,
    SWAP,
    RELOCATE,
    SIZE
  };
  double try_count[neighborhood_options::SIZE]{};
  double success_count[neighborhood_options::SIZE]{};
  std::random_device rand_;
  std::mt19937 gen_;
  std::uniform_real_distribution<double> dist_;

  StepResult perform2opt(const std::shared_ptr<TspIndividualStructured> &individual, const std::shared_ptr<SASchedule> &schedule, const std::shared_ptr<TspIndividualStructured> &best_individual);
  StepResult performSwap(const std::shared_ptr<TspIndividualStructured> &individual, const std::shared_ptr<SASchedule> &schedule, const std::shared_ptr<TspIndividualStructured> &best_individual);
  StepResult performRelocate(const std::shared_ptr<TspIndividualStructured> &individual, const std::shared_ptr<SASchedule> &schedule, const std::shared_ptr<TspIndividualStructured> &best_individual);

  neighborhood_options selectNeighborhoodOption();
  void success(neighborhood_options neighborhood);
  void failure(neighborhood_options neighborhood);

public:
  TspSAStep();
  StepResult step(const std::shared_ptr<Individual> &individual, const std::shared_ptr<SASchedule> &schedule, const std::shared_ptr<Individual> &best_individual) override;
  void reset(const std::shared_ptr<Individual> &individual) override;
};