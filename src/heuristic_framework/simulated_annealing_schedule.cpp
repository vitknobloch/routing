#include "heuristic_framework/simulated_annealing_schedule.h"
#include <cassert>
#include <complex>
#include <cstdlib>
#include <iostream>

#define VEHICLE_COEFFICIENT 200
#define CONSTRAINT_COEFFICIENT 50
SASchedule::SASchedule(
    std::function<double(double)> time_to_temp_function,
    std::function<double(double)> temp_to_vehicle_coeff_function,
    std::function<double(double)> temp_to_constraint_coeff_function) : rand_(), gen_(rand_()), dist_(0, 1) {
  time_to_temp_function_ = std::move(time_to_temp_function);
  temp_to_vehicle_coeff_function_ = std::move(temp_to_vehicle_coeff_function);
  temp_to_constraint_coeff_function_ = std::move(temp_to_constraint_coeff_function);
  start_time_ = std::chrono::steady_clock::now();
  assert(time_to_temp_function_ != nullptr && temp_to_constraint_coeff_function_ != nullptr && temp_to_vehicle_coeff_function_ != nullptr);
}

void SASchedule::registerResult(const StepResult &result) {

}

void SASchedule::reset() {
  start_time_ = std::chrono::steady_clock::now();
}

bool SASchedule::shouldAcceptSolution(const FitnessDiff &diff) {
  assert(diff.constraints > 0 || diff.vehicles > 0 || diff.fitness >= 0);

  const double time_diff = std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time_).count();
  const double temp = time_to_temp_function_(time_diff);
  double total_diff = diff.fitness +
                      temp_to_vehicle_coeff_function_(temp) * diff.vehicles +
                      temp_to_constraint_coeff_function_(temp) * diff.constraints;

  const double accept_prob = std::exp(-total_diff / temp);
  const bool accepted = dist_(gen_) < accept_prob;
  //std::cerr << (accepted ? "+ " : "") << "Accept prob: " << accept_prob << "\tTemp: " << temp << "\ttoal_diff: "<< total_diff << std::endl;
  return accepted;
}

