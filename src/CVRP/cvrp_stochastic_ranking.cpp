#include "CVRP/cvrp_stochastic_ranking.h"
#include <cassert>
#include <iostream>
CvrpStochasticRanking::CvrpStochasticRanking(
    const std::shared_ptr<RoutingInstance> &instance,
    const std::shared_ptr<Mutation> &mutation,
    const std::shared_ptr<Crossover> &crossover, uint population_size,
    uint tournament_size, double fitness_comp_prob) {
  instance_ = instance;
  best_solution_ = nullptr;
  terminate_ = false;
  matrix_ = instance->getMatrix().get();
  stochastic_ranking_ = nullptr;
  mutation_ = mutation;
  crossover_ = crossover;
  population_size_ = population_size;
  tournament_size_ = tournament_size;
  fitness_comp_prob_ = fitness_comp_prob;
}

void CvrpStochasticRanking::run() {
  std::random_device rand;
  std::mt19937 gen(rand());

  auto initial_population = std::make_shared<PopulationStochasticRanking>(fitness_comp_prob_);

  for(uint i = 0; i < population_size_; i++){
    auto individual = std::make_shared<CvrpIndividual>(instance_.get());
    individual->initialize();
    std::shuffle(individual->data().begin(), individual->data().end(), gen);
    individual->evaluate();
    initial_population->addIndividual(individual);
  }

  const auto &best_individual = initial_population->getBestIndividual();
  const auto &best_individual_cast = std::static_pointer_cast<CvrpIndividual>(best_individual);
  auto solution = best_individual_cast->convertSolution();
  if(checkBetterSolution(solution)){
    portfolio_->acceptSolution(solution);
  }

  assert(stochastic_ranking_ != nullptr);
  stochastic_ranking_->run(initial_population);
}

void CvrpStochasticRanking::initialize(HeuristicPortfolio *portfolio) {
  portfolio_ = portfolio;
  terminate_ = false;
  std::shared_ptr<Callbacks> callbacks = std::make_shared<Callbacks>();
  callbacks->setTerminationCondition([this]() -> bool {
    return terminate_;
  });
  callbacks->addNewBestSolutionCallback([this](const std::shared_ptr<Individual> &individual) {
    auto individual_ = std::static_pointer_cast<CvrpIndividual>(individual);
    auto solution = individual_->convertSolution();
    if(checkBetterSolution(solution))
      sendSolution(solution);
  });
  stochastic_ranking_ = std::make_shared<StochasticRanking>(callbacks, mutation_, crossover_, tournament_size_);
}

void CvrpStochasticRanking::terminate() {
  terminate_.store(true);
}

void CvrpStochasticRanking::acceptSolution(std::shared_ptr<Solution> solution) {
  if(checkBetterSolution(solution)){
    auto individual = std::make_shared<CvrpIndividual>(instance_.get(), solution);
    stochastic_ranking_->acceptOutsideSolution(individual);
  }
}

void CvrpStochasticRanking::sendSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution == nullptr || portfolio_ == nullptr)
    return;
  portfolio_->acceptSolution(solution);
}

bool CvrpStochasticRanking::checkBetterSolution(
    const std::shared_ptr<Solution> &solution) {
  if(solution->objective < 0) { // integer overflow
    std::cerr << "Integer overflow encountered in CVRP stochastic ranking solution value" << std::endl;
    return false;
  }
  std::lock_guard<std::recursive_mutex> lock(solution_mutex_);
  if(best_solution_ == nullptr || solution->objective < best_solution_->objective){
    best_solution_ = solution;
    return true;
  }
  return false;
}
