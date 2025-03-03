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
  terminate_ = false;
  best_solution_ = nullptr;
}

void OptalComms::run() {
  while(!terminate_){
    std::string solution_string;
    std::shared_ptr<Solution> new_solution = nullptr;

    if(std::getline(std::cin, solution_string)){
      //received solution from Optal
      new_solution = serializer_->parseSolution(solution_string);
      if(new_solution == nullptr)
        continue;

      std::lock_guard<std::mutex> lock(solution_lock_);
      if (best_solution_ == nullptr || new_solution->objective < best_solution_->objective) {
        best_solution_ = new_solution;
      }
      else
        new_solution = nullptr;
    }
    else{
      //optal closed Pipe
      if(portfolio_ != nullptr)
        portfolio_->terminate();
      this->terminate();
    }

    //received solution is BSF solution
    sendSolution(new_solution);
  }
}

void OptalComms::acceptSolution(std::shared_ptr<Solution> solution) {
  if(!solution->feasible)
    return;
  std::lock_guard<std::mutex> lock(solution_lock_);
  if(best_solution_ == nullptr || solution->objective < best_solution_->objective){
    best_solution_ = solution;
    const std::string solution_string = serializer_->serializeSolution(solution);
    std::cout << solution_string << std::endl;
  }
}

void OptalComms::terminate() {
  terminate_ = true;
  portfolio_ = nullptr;
}

inline void OptalComms::sendSolution(const std::shared_ptr<Solution>& solution) {
  if(solution == nullptr || portfolio_ == nullptr)
    return;
  portfolio_->acceptSolution(solution);
}
