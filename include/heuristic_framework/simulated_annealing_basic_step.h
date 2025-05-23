#pragma once

#include "individual.h"
#include "simulated_annealing_basic_schedule.h"

using StepResult = SABasicScheduleMemory::StepResult;

class SABasicStep{
public:

  virtual StepResult step(std::shared_ptr<Individual> &individual, const std::shared_ptr<SABasicSchedule> &schedule) = 0;
  virtual void reset(const std::shared_ptr<Individual> &individual) = 0;
};
