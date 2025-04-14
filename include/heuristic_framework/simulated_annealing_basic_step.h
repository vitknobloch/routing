//
// Created by knoblvit on 13.4.25.
//

#pragma once

#include "individual.h"
#include "simulated_annealing_basic_schedule.h"

class SABasicStep{
public:

  virtual SABasicSchedule::StepResult step(const std::shared_ptr<Individual> &individual, const std::shared_ptr<SABasicSchedule>) = 0;
  virtual void reset(const std::shared_ptr<Individual> &individual) = 0;
};
