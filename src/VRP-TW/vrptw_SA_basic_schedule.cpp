#include "VRP-TW/vrptw_SA_basic_schedule.h"
#include "heuristic_framework/basic_schedule_memory.h"
#include <cmath>

VrptwSABasicSchedule::VrptwSABasicSchedule(uint memory_size,
                                           double improve_to_accept_ratio) : memory_(memory_size), target_improve_to_accept_ratio_(improve_to_accept_ratio), rand_(), gen_(rand_()), dist_(0, 1){
  accept_probability_ = 0.5;
}
void VrptwSABasicSchedule::registerResult(const StepResult &result) {
  memory_.push(result);
}
void VrptwSABasicSchedule::reset() {
  accept_probability_ = 0.5;
  memory_.clear();
}
bool VrptwSABasicSchedule::shouldAcceptSolution() {
  if(memory_.improvedCount() == 0 && memory_.acceptedCount() == 0)
    return true;

  if(memory_.acceptedCount() == 0)
    return true;

  double improve_to_accept_ratio = (double)memory_.improvedCount() / memory_.acceptedCount();
  accept_probability_ = std::pow(accept_probability_, target_improve_to_accept_ratio_ / improve_to_accept_ratio);
  accept_probability_ = std::max(accept_probability_, 1e-4);
  accept_probability_ = std::min(accept_probability_, 1.0-1e-4);

  return dist_(gen_) < accept_probability_;
}