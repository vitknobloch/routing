//
// Created by knoblvit on 9.2.25.
//
#include "VRP-TW/setup.h"
#include <iostream>

#include "VRP-TW/vrptw_SA_basic.h"
#include "VRP-TW/vrptw_SA_basic_schedule.h"
#include "VRP-TW/vrptw_SA_basic_step.h"
#include "VRP-TW/vrptw_exhaustive_local_search.h"
#include "VRP-TW/vrptw_memetic.h"
#include "VRP-TW/vrptw_mutation_reinsert.h"
#include "VRP-TW/vrptw_neighborhood.h"
#include "VRP-TW/vrptw_pmx_crossover_structured.h"
#include "common/optal_comms.h"
#include "common/routing_instance.h"
#include "heuristic_framework/tournament_selection.h"
#include "heuristic_framework/truncation_replacement.h"

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
    else if(heur_config["type"] == "simulated_annealing"){
      auto step = std::make_shared<VrptwSABasicStep>();
      auto schedule = std::make_shared<VrptwSABasicSchedule>(10000, 20.0);
      auto sa = std::make_shared<VrptwSABasic>(instance, step, schedule);
      portfolio->addImprovingHeuristic(sa);
    }
    else if(heur_config["type"] == "memetic_algorithm"){
      auto mutation = std::make_shared<VrptwMutationReinsert>();
      mutation->setMutationRate(0.1);
      auto selection = std::make_shared<TournamentSelection>(3);
      auto crossover = std::make_shared<VrptwPmxCrossoverStructured>();
      crossover->setCrossoverRate(0.8);
      auto neighborhood = std::make_shared<VrptwNeighborhood>();
      auto replacement = std::make_shared<TruncationReplacement>();
      auto memetic_algorithm = std::make_shared<VrptwMemetic>(
          instance,
          neighborhood,
          mutation,
          selection,
          crossover,
          replacement,
          10
      );
      portfolio->addImprovingHeuristic(memetic_algorithm);
    }
  }

  std::cerr << "Heuristic vehicle count " << instance->getVehicleCount() << std::endl;


  return portfolio;
}
