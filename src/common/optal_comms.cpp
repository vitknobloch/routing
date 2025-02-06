//
// Created by knoblvit on 6.2.25.
//
#include "common/optal_comms.h"
#include <iostream>
#include <mutex>

OptalComms::OptalComms(const std::shared_ptr<SolutionSerializer> &serializer){
  serializer_ = serializer;
  portfolio_ = nullptr;
  terminate_ = false;
  best_solution_ = nullptr;

}

void OptalComms::initialize(HeuristicPortfolio *portfolio){
  portfolio_ = portfolio;
}

void OptalComms::run() {
  while(!terminate_){
    std::string solution_string;
    std::shared_ptr<Solution> new_solution = nullptr;
    if(std::getline(std::cin, solution_string)){ //received solution from Optal
      new_solution = serializer_->parseSolution(solution_string);
      std::lock_guard<std::mutex> lock(solution_lock_);
      if (best_solution_ == nullptr || new_solution->objective < best_solution_->objective) {
        best_solution_ = new_solution;
      }
      else
        new_solution = nullptr;
    }
    else{ //optal closed Pipe
      terminate_ = true;
      portfolio_->terminate();
    }

    //received solution is BSF solution
    if(new_solution != nullptr)
      portfolio_->acceptSolution(new_solution);
  }
}

void OptalComms::acceptSolution(std::shared_ptr<Solution> solution) {
  std::lock_guard<std::mutex> lock(solution_lock_);
  if(best_solution_ == nullptr || solution->objective < best_solution_->objective){
    best_solution_ = solution;
  }
}
void OptalComms::terminate() {
  terminate_ = true;
}
