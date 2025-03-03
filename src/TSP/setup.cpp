//
// Created by knoblvit on 9.2.25.
//
#include "TSP/setup.h"
#include <iostream>

#include "TSP/tsp_local_search.h"
#include "TSP/tsp_genetic_algorithm.h"
#include "TSP/tsp_mutation_2opt.h"
#include "TSP/tsp_pmx_crossover.h"
#include "heuristic_framework/truncation_replacement.h"
#include "heuristic_framework/tournament_selection.h"
#include "common/optal_comms.h"
#include "common/routing_instance.h"

std::shared_ptr<HeuristicPortfolio>
SetupTSP::preparePortfolio(const JSON &config, const char *instance_filename) {
  auto portfolio = std::make_shared<HeuristicPortfolio>();
  auto instance = std::make_shared<RoutingInstance>();
  instance->loadTSPlibInstance(instance_filename);

  //Optal communication thread (it has heuristic interface / optal is basically one of the heuristics)
  auto serializer = std::make_shared<SolutionSerializer>();
  auto optalComms = std::make_shared<OptalComms>(serializer);
  portfolio->addImprovingHeuristic(optalComms);

  for(int h = 0; h < config.size(); h++){
    auto heur_config = config[h];
    if(!heur_config.contains("type"))
        std::cerr << "Heuristic config doesn't contain type." << std::endl;
    // Local-search initialization
    if(heur_config["type"] == "local_search"){
      auto mutation = std::make_shared<TspMutation2opt>(instance->getMatrix().get());
      auto localSearch = std::make_shared<TspLocalSearch>(instance, mutation);
      portfolio->addImprovingHeuristic(localSearch);
    }
    // Genetic-algorithm initialization
    else if(heur_config["type"] == "genetic_algorithm"){
      auto mutation = std::make_shared<TspMutation2opt>(instance->getMatrix().get());
      mutation->setMutationRate(1.0);
      auto selection = std::make_shared<TournamentSelection>(3);
      auto crossover = std::make_shared<TspPmxCrossover>();
      crossover->setCrossoverRate(1.0);
      auto replacement = std::make_shared<TruncationReplacement>();
      auto geneticAlgorithm = std::make_shared<TspGeneticAlgorithm>(
          instance,
          mutation,
          selection,
          crossover,
          replacement,
          10
          );
      portfolio->addImprovingHeuristic(geneticAlgorithm);
    }
  }
  return portfolio;
}
