#pragma once

#include "heuristic_framework/simulated_annealing_step.h"
#include "CVRP/cvrp_structured_individual.h"
#include <random>

using stepResult = SABasicScheduleMemory::StepResult;

class CvrpSAStep : public SAStep{
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

  StepResult perform2opt(const std::shared_ptr<CvrpIndividualStructured> &individual, const std::shared_ptr<SASchedule> &schedule, const std::shared_ptr<CvrpIndividualStructured> &best_individual);
  StepResult performExchange(const std::shared_ptr<CvrpIndividualStructured> &individual, const std::shared_ptr<SASchedule> &schedule, const std::shared_ptr<CvrpIndividualStructured> &best_individual);
  StepResult performRelocate(const std::shared_ptr<CvrpIndividualStructured> &individual, const std::shared_ptr<SASchedule> &schedule, const std::shared_ptr<CvrpIndividualStructured> &best_individual);
  StepResult performCross(const std::shared_ptr<CvrpIndividualStructured> &individual, const std::shared_ptr<SASchedule> &schedule, const std::shared_ptr<CvrpIndividualStructured> &best_individual);

  uint randomSelectRoute(const std::shared_ptr<CvrpIndividualStructured> &individual, uint min_length = 0, int exclude_route = -1);
  CvrpRouteSegment randomSelectSegment(const std::shared_ptr<CvrpIndividualStructured> &individual, uint route, uint min_length = 0, bool include_end = true);

  neighborhood_options selectNeighborhoodOption();
  void success(neighborhood_options neighborhood);
  void failure(neighborhood_options neighborhood);

public:
  CvrpSAStep();
  StepResult step(const std::shared_ptr<Individual> &individual, const std::shared_ptr<SASchedule> &schedule, const std::shared_ptr<Individual> &best_individual) override;
  void reset(const std::shared_ptr<Individual> &individual) override;
};