//
// Created by knoblvit on 9.2.25.
//
#include "VRP-TW/setup.h"
#include <iostream>

#include "common/routing_instance.h"
#include "common/optal_comms.h"
#include "VRP-TW/vrptw_neighborhood.h"
#include "VRP-TW/vrptw_exhaustive_local_search.h"


std::shared_ptr<HeuristicPortfolio>
SetupVRPTW::preparePortfolio(const JSON &config, const char *instance_filename) {
  auto portfolio = std::make_shared<HeuristicPortfolio>();
  auto instance = std::make_shared<RoutingInstance>();
  instance->loadSolomonInstance(instance_filename);

  //Optal communication thread (it has heuristic interface / optal is basically one of the heuristics)
  auto serializer = std::make_shared<SolutionSerializer>();
  auto optalComms = std::make_shared<OptalComms>(serializer);
  portfolio->addImprovingHeuristic(optalComms);

  for(uint h = 0; h < config.size(); h++){
    auto heur_config = config[h];
    if(!heur_config.contains("type"))
      std::cerr << "Heuristic config doesn't contain type." << std::endl;
    if(heur_config["type"] == "exhaustive_local_search"){
      auto neighborhood = std::make_shared<VrptwNeighborhood>();
      auto localSearch = std::make_shared<VrptwExhaustiveLocalSearch>(instance, neighborhood);
      portfolio->addImprovingHeuristic(localSearch);
    }
  }

  std::cerr << "Heuristic vehicle count " << instance->getVehicleCount() << std::endl;


  return portfolio;
}
