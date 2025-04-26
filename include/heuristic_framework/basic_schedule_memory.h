#pragma once

#include <vector>

using uint = unsigned int;

class SABasicScheduleMemory{
public:
  enum StepResult {
    IMPROVED /** Better solution found */,
    ACCEPTED /** Worse solution accepted */,
    UNACCEPTED /** Worse solution not accepted */
  };

private:
  std::vector<char> memory_;
  uint memory_size_;
  uint memory_ptr_;
  /* Count of accepted worse solution in memory */
  uint accept_count_;
  /* Count of rejected worse solution in memory */
  uint unaccept_count_;
  /* Count of better solutions in memory*/
  uint improve_count_;

public:
  explicit SABasicScheduleMemory(const uint &memory_size);
  [[nodiscard]] uint size() const;
  uint capacity();
  void push(const StepResult &stepResult);
  void clear();
  bool full();
  [[nodiscard]] inline uint acceptedCount() const { return accept_count_;};
  [[nodiscard]] inline uint unacceptedCount() const { return unaccept_count_;};
  [[nodiscard]] inline uint improvedCount() const {return improve_count_;};
};