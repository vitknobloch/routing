#pragma once

#include "individual.h"
#include "simulated_annealing_schedule.h"

using StepResult = SABasicScheduleMemory::StepResult;

class SAStep{
public:
  virtual StepResult step(const std::shared_ptr<Individual> &individual, const std::shared_ptr<SASchedule> &schedule, const std::shared_ptr<Individual> &best_individual) = 0;
  virtual void reset(const std::shared_ptr<Individual> &individual) = 0;
};