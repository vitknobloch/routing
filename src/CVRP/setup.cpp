//
// Created by knoblvit on 9.2.25.
//
#include "CVRP//setup.h"
#include <iostream>

#include "CVRP/cvrp_local_search.h"
#include "CVRP/cvrp_mutation_random.h"
#include "CVRP/cvrp_pmx_crossover.h"
#include "CVRP/cvrp_stochastic_ranking.h"
#include "common/optal_comms.h"
#include "common/routing_instance.h"

std::shared_ptr<HeuristicPortfolio>
SetupCVRP::preparePortfolio(const JSON &config, const char *instance_filename) {
  auto portfolio = std::make_shared<HeuristicPortfolio>();
  auto instance = std::make_shared<RoutingInstance>();
  instance->loadTSPlibInstance(instance_filename);

  //Optal communication thread (it has heuristic interface / optal is basically one of the heuristics)
  auto serializer = std::make_shared<SolutionSerializer>();
  auto optalComms = std::make_shared<OptalComms>(serializer);
  portfolio->addImprovingHeuristic(optalComms);

  for(uint h = 0; h < config.size(); h++){
    auto heur_config = config[h];
    if(!heur_config.contains("type"))
      std::cerr << "Heuristic config doesn't contain type." << std::endl;
    if(heur_config["type"] == "local_search"){
      auto mutation = std::make_shared<CvrpMutationRandom>(instance);
      auto localSearch = std::make_shared<CvrpLocalSearch>(instance, mutation);
      portfolio->addImprovingHeuristic(localSearch);
    }
    else if(heur_config["type"] == "stochastic_ranking"){
      auto mutation = std::make_shared<CvrpMutationRandom>(instance);
      mutation->setMutationRate(1.0);
      auto crossover = std::make_shared<CvrpPmxCrossover>();
      crossover->setCrossoverRate(1.0);
      auto stochasticRanking = std::make_shared<CvrpStochasticRanking>(
          instance,
          mutation,
          crossover,
          10, // Population size
          3, // tournament size
          0.55 // fitness compare probability
          );
      portfolio->addImprovingHeuristic(stochasticRanking);
    }
  }

  std::cerr << "Heuristic vehicle count " << instance->getVehicleCount() << std::endl;

  return portfolio;
}
