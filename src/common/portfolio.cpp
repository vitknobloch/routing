//
// Created by knoblvit on 6.2.25.
//
#include "common/portfolio.h"

#include <iostream>
#include <thread>

HeuristicPortfolio::HeuristicPortfolio() :
  improving_heuristics_(), constructive_heuristics_(), best_solution_(nullptr) {

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
  std::vector<std::thread> threads;
  threads.reserve(constructive_heuristics_.size() + improving_heuristics_.size());
  // start constructive heuristics
  for(auto& heur: constructive_heuristics_){
    threads.emplace_back([this, heur]()->void{ startThread(heur);});
  }

  // start improving heuristics
  for(auto& heur: improving_heuristics_){
    threads.emplace_back([this, heur]()->void{ startThread(heur);});
  }

  //wait for threads to finish
  for(auto &thread : threads){
    thread.join();
  }
}

void HeuristicPortfolio::startThread(std::shared_ptr<Heuristic> heuristic) {
  heuristic->initialize(this);
  heuristic->run();
}

void HeuristicPortfolio::terminate() {
  for(auto& heur: constructive_heuristics_){
    heur->terminate();
  }

  for(auto& heur: improving_heuristics_){
    heur->terminate();
  }
}

void HeuristicPortfolio::acceptSolution(std::shared_ptr<Solution> solution) {
  std::lock_guard<std::mutex> lock(solution_lock_);
  if(best_solution_ == nullptr || solution->objective < best_solution_->objective){
    best_solution_ = solution;
  }
  std::cerr << best_solution_->objective << std::endl;
  sendSolution();
}

void HeuristicPortfolio::sendSolution() {
  for(auto& heur: improving_heuristics_){
    heur->acceptSolution(best_solution_);
  }
}
