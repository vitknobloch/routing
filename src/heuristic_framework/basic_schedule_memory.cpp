#include "heuristic_framework/basic_schedule_memory.h"
#include <cassert>

SABasicScheduleMemory::SABasicScheduleMemory(const uint &memory_size) {
  memory_ = std::vector<char>(memory_size, 0);
  memory_size_ = 0;
  memory_ptr_ = 0;
  accept_count_ = 0;
  unaccept_count_ = 0;
  improve_count_ = 0;
}

inline uint SABasicScheduleMemory::size() const {
  return memory_size_;
}

inline uint SABasicScheduleMemory::capacity() { return memory_.size(); }

void SABasicScheduleMemory::push(
    const SABasicScheduleMemory::StepResult &stepResult) {
  if(memory_size_ < memory_.size()){
    switch (stepResult) {
    case IMPROVED:
      improve_count_++;
      memory_[memory_ptr_] = 1;
      break;
    case ACCEPTED:
      accept_count_++;
      memory_[memory_ptr_] = 2;
      break;
    case UNACCEPTED:
      unaccept_count_++;
      memory_[memory_ptr_] = 3;
      break;
    }
    memory_size_++;
    memory_ptr_ = (memory_ptr_ + 1) % memory_.size();
    return;
  }

  switch (memory_[memory_ptr_]) {
  case 0:
    break;
  case 1:
    improve_count_--;
    break;
  case 2:
    accept_count_--;
    break;
  case 3:
    unaccept_count_--;
    break;
  default:
    assert(false);
  }

  switch (stepResult) {
  case IMPROVED:
    improve_count_++;
    memory_[memory_ptr_] = 1;
    break;
  case ACCEPTED:
    accept_count_++;
    memory_[memory_ptr_] = 2;
    break;
  case UNACCEPTED:
    unaccept_count_++;
    memory_[memory_ptr_] = 3;
    break;
  }
  memory_ptr_ = (memory_ptr_ + 1) % memory_.size();
}

void SABasicScheduleMemory::clear() {
  memory_ptr_ = 0;
  memory_size_ = 0;
  accept_count_ = 0;
  unaccept_count_ = 0;
  improve_count_ = 0;
}

inline bool SABasicScheduleMemory::full() { return memory_size_ >= memory_.size();}
