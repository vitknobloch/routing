#pragma once

#include "basic_schedule_memory.h"
#include "simulated_annealing_fitness_diff.h"
#include <chrono>
#include <functional>
#include <random>

using StepResult = SABasicScheduleMemory::StepResult;

class SASchedule{
private:
    std::function<double(double)> time_to_temp_function_;
    std::function<double(double)> temp_to_vehicle_coeff_function_;
    std::function<double(double)> temp_to_constraint_coeff_function_;

    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    std::random_device rand_;
    std::mt19937 gen_;
    std::uniform_real_distribution<double> dist_;

public:
  explicit SASchedule(
      std::function<double(double)> time_to_temp_function,
      std::function<double(double)> temp_to_vehicle_coeff_function,
      std::function<double(double)> temp_to_constraint_coeff_function);

  void registerResult(const StepResult &result);
  void reset();
  bool shouldAcceptSolution(const FitnessDiff &diff);
};