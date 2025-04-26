#pragma once
#include <chrono>
#include <fstream>
#include "common/solution.h"

class ObjectiveValueLogger{
private:
  std::chrono::time_point<std::chrono::steady_clock> start_time_;
  std::ofstream file_;

  double getSecondsElapsed();

public:
  explicit ObjectiveValueLogger(const char *log_filename);
  ~ObjectiveValueLogger();
  void startClock();
  void log(const Solution &solution);
  void closeFile();
};
