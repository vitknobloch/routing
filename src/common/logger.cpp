//
// Created by knoblvit on 12.2.25.
//
#include "common/logger.h"
#include <iostream>

ObjectiveValueLogger::ObjectiveValueLogger(const char *log_filename) {
  file_ = std::ofstream(log_filename);
  if(!file_){
    std::cerr << "Error creating log file" << std::endl;
    exit(102);
  }
  startClock();
}

ObjectiveValueLogger::~ObjectiveValueLogger() {
  closeFile();
}

void ObjectiveValueLogger::closeFile() {
  if(file_ && file_.is_open()){
    file_.close();
  }
  file_ = (std::ofstream) nullptr;
}

void ObjectiveValueLogger::startClock() {
  start_time_ = std::chrono::steady_clock::now();
}

double ObjectiveValueLogger::getSecondsElapsed() {
  std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start_time_;
  return elapsed.count();
}

void ObjectiveValueLogger::log(const Solution &solution) {
  if(!solution.feasible){
    return;
  }
  if(file_ && file_.is_open()){
    if(solution.used_vehicles != 0)
      file_ << getSecondsElapsed() << " " << solution.objective << " " << solution.used_vehicles << std::endl;
    else
      file_ << getSecondsElapsed() << " " << solution.objective << std::endl;
  }else{
    std::cerr << "Log file is closed when trying to log a solution." << std::endl;
  }
}
