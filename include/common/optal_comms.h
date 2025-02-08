//
// Created by knoblvit on 6.2.25.
//

#pragma once
#include "heuristic.h"
#include "serializer.h"
#include <atomic>
#include <mutex>

class OptalComms: public Heuristic{
private:
  HeuristicPortfolio *portfolio_;
  std::atomic<bool> terminate_{};
  std::shared_ptr<Solution> best_solution_;
  std::shared_ptr<SolutionSerializer> serializer_;
  std::mutex solution_lock_;

  void sendSolution(const std::shared_ptr<Solution>& solution);

public:
  explicit OptalComms(const std::shared_ptr<SolutionSerializer> &serializer);
  void initialize(HeuristicPortfolio *portfolio) override;
  void run() override;
  void acceptSolution(std::shared_ptr<Solution> solution) override;
  void terminate() override;
};
