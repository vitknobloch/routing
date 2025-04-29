#pragma once

#include "heuristic_framework/simulated_annealing_basic_step.h"
#include "VRP-TW/vrptw_structured_individual.h"
#include <random>

using StepResult = SABasicScheduleMemory::StepResult;

class VrptwSABasicStep : public SABasicStep{
private:
  enum neighborhood_options{
    TWO_OPT,
    EXCHANGE,
    RELOCATE,
    CROSS,
    SIZE
  };
  double try_count[neighborhood_options::SIZE]{};
  double success_count[neighborhood_options::SIZE]{};
  std::random_device rand_;
  std::mt19937 gen_;
  std::uniform_real_distribution<double> dist_;

  StepResult perform2opt(const std::shared_ptr<VrptwIndividualStructured> &individual, const std::shared_ptr<SABasicSchedule> &schedule);
  StepResult performExchange(const std::shared_ptr<VrptwIndividualStructured> &individual, const std::shared_ptr<SABasicSchedule> &schedule);
  StepResult performRelocate(const std::shared_ptr<VrptwIndividualStructured> &individual, const std::shared_ptr<SABasicSchedule> &schedule);
  StepResult performCross(const std::shared_ptr<VrptwIndividualStructured> &individual, const std::shared_ptr<SABasicSchedule> &schedule);

  uint randomSelectRoute(const std::shared_ptr<VrptwIndividualStructured> &individual, uint min_length = 0, int exclude_route = -1);
  VrptwRouteSegment randomSelectSegment(const std::shared_ptr<VrptwIndividualStructured> &individual, uint route, uint min_length = 0, bool include_end = true);

  neighborhood_options selectNeighborhoodOption();
  void success(neighborhood_options neighborhood);
  void failure(neighborhood_options neighborhood);

public:
  VrptwSABasicStep();
  StepResult step(std::shared_ptr<Individual> &individual, const std::shared_ptr<SABasicSchedule> &schedule) override;
  void reset(const std::shared_ptr<Individual> &individual) override;
};