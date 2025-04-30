#include "VRP-TW/vrptw_simulated_annealing.h"
#include <cassert>
#include <iostream>

VrptwSimulatedAnnealing::VrptwSimulatedAnnealing(
    const std::shared_ptr<RoutingInstance> &instance,
    const std::shared_ptr<SAStep> &step,
    const std::shared_ptr<SASchedule> schedule) {
  instance_ = instance;
  step_ = step;
  schedule_ = schedule;
  best_solution_ = nullptr;
  terminate_ = false;
  simulated_annealing_ = nullptr;
  portfolio_ = nullptr;
  assert(instance != nullptr && step != nullptr && schedule != nullptr);
}

void VrptwSimulatedAnnealing::sendSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution == nullptr || portfolio_ == nullptr)
    return;

  uint count = 0;
  for(const auto &r: solution->routes){
    if(r.route_nodes.size() > 2)
      count++;
  }

  portfolio_->acceptSolution(solution);
}

bool VrptwSimulatedAnnealing::checkBetterSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution->objective < 0) { // integer overflow
    std::cerr << "Integer overflow encountered in CVRP exhaustive local search solution value" << std::endl;
    return false;
  }
  std::lock_guard<std::recursive_mutex> lock(solution_mutex_);
  if(best_solution_ == nullptr || solution->betterThan(*best_solution_)){
    best_solution_ = solution;
    return true;
  }
  return false;
}

void VrptwSimulatedAnnealing::initialize(HeuristicPortfolio *portfolio) {
  portfolio_ = portfolio;
  terminate_ = false;
  std::shared_ptr<Callbacks> callbacks = std::make_shared<Callbacks>();
  callbacks->setTerminationCondition([this]() -> bool {
    return terminate_;
  });
  callbacks->addNewBestSolutionCallback([this](const std::shared_ptr<Individual> &individual) {
    auto individual_ = std::static_pointer_cast<VrptwIndividualStructured>(individual);
    auto solution = individual_->convertSolution();
    //std::cerr << solution->objective << " " << individual->getTotalConstraintViolation() << std::endl;
    sendSolution(solution);
  });
  simulated_annealing_ = std::make_shared<SimulatedAnnealing>(callbacks, step_, schedule_);
}

void VrptwSimulatedAnnealing::run() {
  auto initialSolution = std::make_shared<VrptwIndividualStructured>(instance_.get());
  initialSolution->smartInitialize();
  initialSolution->evaluate();
  auto solution = initialSolution->convertSolution();
  portfolio_->acceptSolution(solution);
  simulated_annealing_->run(initialSolution);
}

void VrptwSimulatedAnnealing::terminate() {
  terminate_.store(true);
}

void VrptwSimulatedAnnealing::acceptSolution(
    std::shared_ptr<Solution> solution) {
  if(checkBetterSolution(solution)){
    auto individual = std::make_shared<VrptwIndividualStructured>(instance_.get(), solution);
    simulated_annealing_->acceptOutsideSolution(individual);
  }
}
