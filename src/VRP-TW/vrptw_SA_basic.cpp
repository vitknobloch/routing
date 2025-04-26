#include "VRP-TW/vrptw_SA_basic.h"
#include <cassert>
#include <iostream>

VrptwSABasic::VrptwSABasic(const std::shared_ptr<RoutingInstance> &instance,
                           const std::shared_ptr<SABasicStep> &step,
                           const std::shared_ptr<SABasicSchedule> schedule) {
  instance_ = instance;
  step_ = step;
  schedule_ = schedule;
  best_solution_ = nullptr;
  terminate_ = false;
  sa_basic_ = nullptr;
  assert(instance != nullptr && step != nullptr && schedule != nullptr);
}

void VrptwSABasic::sendSolution(const std::shared_ptr<Solution> &solution) {
  if(solution == nullptr || portfolio_ == nullptr)
    return;


  uint count = 0;
  for(const auto &r: solution->routes){
    if(r.route_nodes.size() > 2)
      count++;
  }
  std::cerr << "solution routes: " << count << std::endl;


  portfolio_->acceptSolution(solution);
}

bool VrptwSABasic::checkBetterSolution(
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

void VrptwSABasic::initialize(HeuristicPortfolio *portfolio) {
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
  sa_basic_ = std::make_shared<SimulatedAnnealingBasic>(callbacks, step_, schedule_);
}

void VrptwSABasic::terminate() {
  terminate_.store(true);
}

void VrptwSABasic::acceptSolution(std::shared_ptr<Solution> solution) {
  if(checkBetterSolution(solution)){
    auto individual = std::make_shared<VrptwIndividualStructured>(instance_.get(), solution);
    sa_basic_->acceptOutsideSolution(individual);
  }
}

void VrptwSABasic::run() {
  auto initialSolution = std::make_shared<VrptwIndividualStructured>(instance_.get());
  initialSolution->smartInitialize();
  initialSolution->evaluate();
  auto solution = initialSolution->convertSolution();
  portfolio_->acceptSolution(solution);
  sa_basic_->run(initialSolution);
}
