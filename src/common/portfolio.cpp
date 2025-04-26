#include "common/portfolio.h"

#include <iostream>
#include <thread>

HeuristicPortfolio::HeuristicPortfolio() :
  improving_heuristics_(), constructive_heuristics_(),
  logger_(nullptr), best_solution_(nullptr) {

}
void HeuristicPortfolio::addImprovingHeuristic(
    const std::shared_ptr<Heuristic>& heuristic) {
  improving_heuristics_.push_back(heuristic);
}

void HeuristicPortfolio::addConstructiveHeuristic(
    const std::shared_ptr<Heuristic>& heuristic) {
  constructive_heuristics_.push_back(heuristic);
}

void HeuristicPortfolio::start() {
  initializeThreads();
  if(logger_ != nullptr){
    logger_->startClock();
  }
  runThreads();
}

void HeuristicPortfolio::terminate() {
  for(auto& heur: constructive_heuristics_){
    heur->terminate();
  }

  for(auto& heur: improving_heuristics_){
    heur->terminate();
  }
}

void HeuristicPortfolio::acceptSolution(const std::shared_ptr<Solution>& solution) {
  std::lock_guard<std::mutex> lock(solution_lock_);
  if(best_solution_ == nullptr || solution->betterThan(*best_solution_)){
    best_solution_ = solution;
    sendSolution();

    if(logger_ != nullptr)
      logger_->log(*solution);
  }
}

void HeuristicPortfolio::sendSolution() {
  for(auto& heur: improving_heuristics_){
    heur->acceptSolution(best_solution_);
  }
}

void HeuristicPortfolio::setLogger(
    const std::shared_ptr<ObjectiveValueLogger> &logger) {
  logger_ = logger;
}

void HeuristicPortfolio::initializeThreads() {
  std::vector<std::thread> threads;
  threads.reserve(constructive_heuristics_.size() + improving_heuristics_.size());
  // start constructive heuristics
  for(auto& heur: constructive_heuristics_){
    threads.emplace_back([this, heur]()->void{ heur->initialize(this);});
  }

  // start improving heuristics
  for(auto& heur: improving_heuristics_){
    threads.emplace_back([this, heur]()->void{ heur->initialize(this);});
  }

  //wait for threads to finish
  for(auto &thread : threads){
    thread.join();
  }
}

void HeuristicPortfolio::runThreads() {
  std::vector<std::thread> threads;
  threads.reserve(constructive_heuristics_.size() + improving_heuristics_.size());
  // start constructive heuristics
  for(auto& heur: constructive_heuristics_){
    threads.emplace_back([this, heur]()->void{ heur->run();});
  }

  // start improving heuristics
  for(auto& heur: improving_heuristics_){
    threads.emplace_back([this, heur]()->void{ heur->run();});
  }

  //wait for threads to finish
  for(auto &thread : threads){
    thread.join();
  }
}
