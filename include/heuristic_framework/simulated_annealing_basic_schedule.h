//
// Created by knoblvit on 13.4.25.
//

#pragma once

#include "basic_schedule_memory.h"

using StepResult = SABasicScheduleMemory::StepResult;

class SABasicSchedule{

public:

  virtual void registerResult(const StepResult &result) = 0;
  virtual void reset() = 0;
  virtual bool shouldAcceptSolution() = 0;

};