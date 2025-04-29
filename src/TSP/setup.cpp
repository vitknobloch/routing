#include "TSP/setup.h"
#include <iostream>

#include "TSP/tsp_SA_step.h"
#include "TSP/tsp_exhaustive_local_search.h"
#include "TSP/tsp_genetic_algorithm.h"
#include "TSP/tsp_local_search.h"
#include "TSP/tsp_memetic.h"
#include "TSP/tsp_mutation_2opt.h"
#include "TSP/tsp_mutation_double_bridge.h"
#include "TSP/tsp_neighborhood.h"
#include "TSP/tsp_pmx_crossover.h"
#include "TSP/tsp_pmx_crossover_structured.h"
#include "TSP/tsp_simulated_annealing.h"
#include "common/SA_schedule_functions.h"
#include "common/optal_comms.h"
#include "common/routing_instance.h"
#include "heuristic_framework/tournament_selection.h"
#include "heuristic_framework/truncation_replacement.h"

std::shared_ptr<HeuristicPortfolio>
SetupTSP::preparePortfolio(const JSON &config, const char *instance_filename) {
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
    else if(heur_config["type"] == "memetic_algorithm"){
      auto mutation = std::make_shared<TspMutationDoubleBridge>();
      mutation->setMutationRate(0.1);
      auto selection = std::make_shared<TournamentSelection>(3);
      auto crossover = std::make_shared<TspPmxCrossoverStructured>();
      crossover->setCrossoverRate(0.8);
      auto neighborhood = std::make_shared<TspNeighborhood>();
      auto replacement = std::make_shared<TruncationReplacement>();
      auto memetic_algorithm = std::make_shared<TspMemetic>(
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
    else if(heur_config["type"] == "exhaustive_local_search"){
      auto neighborhood = std::make_shared<TspNeighborhood>();
      auto localSearch = std::make_shared<TspExhaustiveLocalSearch>(instance, neighborhood);
      portfolio->addImprovingHeuristic(localSearch);
    }
    else if(heur_config["type"] == "simulated_annealing"){
      auto step = std::make_shared<TspSAStep>();
      auto average_length = getAverageEdgeLength(*instance);
      auto initial_temperature =
          getTemperatureByTargetAcceptanceRate(average_length / 10, 0.5);
      auto final_temperature =
          getTemperatureByTargetAcceptanceRate(average_length / 10, 0.01);
      auto schedule = std::make_shared<SASchedule>(
          getExpSchedule(initial_temperature, final_temperature, 60.0),
          [](double t){return 0;},
          [](double t){return 0;}
          );
      //auto schedule = std::make_shared<SASchedule>([](double t){return 0.01;});
      auto sa = std::make_shared<TspSimulatedAnnealing>(instance, step, schedule);
      portfolio->addImprovingHeuristic(sa);
    }
    else{
      std::cerr << "Unknown heuristic type: " << heur_config["type"] << std::endl;
      exit(101);
    }
  }
  return portfolio;
}
