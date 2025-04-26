#pragma once

#include "heuristic_framework/simulated_annealing_basic_schedule.h"

#include <random>

class VrptwSABasicSchedule: public SABasicSchedule{
private:
  SABasicScheduleMemory memory_;
  /** Ratio between improvement steps and accepting a non-improving move */
  double target_improve_to_accept_ratio_;
  double accept_probability_;
  std::random_device rand_;
  std::mt19937 gen_;
  std::uniform_real_distribution<double> dist_;


public:
  VrptwSABasicSchedule(uint memory_size, double improve_to_accept_ratio);
  void registerResult(const StepResult &result) override;
  void reset() override;
  bool shouldAcceptSolution() override;
};