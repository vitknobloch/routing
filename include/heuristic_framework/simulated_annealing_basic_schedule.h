//
// Created by knoblvit on 13.4.25.
//

#pragma once

class SABasicSchedule{

public:
  enum StepResult {
    IMPROVED /** Better solution found */,
    ACCEPTED /** Worse solution accepted */,
    UNACCEPTED /** Worse solution not accepted */
  };

  virtual void registerResult(const StepResult &result) = 0;
  virtual void reset() = 0;
  virtual bool shouldAcceptSolution() = 0;

};