#include "CVRP//setup.h"
#include <iostream>

#include "CVRP/cvrp_SA_step.h"
#include "CVRP/cvrp_exhaustive_local_search.h"
#include "CVRP/cvrp_memetic.h"
#include "CVRP/cvrp_mutation_random.h"
#include "CVRP/cvrp_mutation_reinsert.h"
#include "CVRP/cvrp_neighborhood.h"
#include "CVRP/cvrp_pmx_crossover.h"
#include "CVRP/cvrp_pmx_crossover_structured.h"
#include "CVRP/cvrp_simulated_annealing.h"
#include "CVRP/cvrp_stochastic_local_search.h"
#include "CVRP/cvrp_stochastic_ranking.h"
#include "common/SA_schedule_functions.h"
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
    if(heur_config["type"] == "stochastic_local_search"){
      auto mutation = std::make_shared<CvrpMutationRandom>(instance);
      auto localSearch = std::make_shared<CvrpStochasticLocalSearch>(instance, mutation);
      portfolio->addImprovingHeuristic(localSearch);
    }
    else if(heur_config["type"] == "exhaustive_local_search"){
      auto neighborhood = std::make_shared<CvrpNeighborhood>();
      auto localSearch = std::make_shared<CvrpExhaustiveLocalSearch>(instance, neighborhood);
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
    else if(heur_config["type"] == "memetic_algorithm"){
      auto mutation = std::make_shared<CvrpMutationReinsert>();
      mutation->setMutationRate(0.1);
      auto selection = std::make_shared<TournamentSelection>(3);
      auto crossover = std::make_shared<CvrpPmxCrossoverStructured>();
      crossover->setCrossoverRate(0.8);
      auto neighborhood = std::make_shared<CvrpNeighborhood>();
      auto replacement = std::make_shared<TruncationReplacement>();
      auto memetic_algorithm = std::make_shared<CvrpMemetic>(
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
    else if(heur_config["type"] == "simulated_annealing"){
      auto step = std::make_shared<CvrpSAStep>();
      auto average_length = getAverageEdgeLength(*instance);
      auto initial_temperature =
          getTemperatureByTargetAcceptanceRate(average_length / 10, 0.5);
      auto final_temperature =
          getTemperatureByTargetAcceptanceRate(average_length / 10, 0.01);
      auto schedule = std::make_shared<SASchedule>(
          getExpSchedule(initial_temperature, final_temperature, 60.0), //temp
          [](double t){return 0;}, //vehicle
          getEquivalentPunishmentFunction(average_length, 15.0) //constraint
      );
      //auto schedule = std::make_shared<SASchedule>([](double t){return 0.01;});
      auto sa = std::make_shared<CvrpSimulatedAnnealing>(instance, step, schedule);
      portfolio->addImprovingHeuristic(sa);
    }
    else{
      std::cerr << "Unknown heuristic type: " << heur_config["type"] << std::endl;
      exit(101);
    }
  }

  std::cerr << "Heuristic vehicle count " << instance->getVehicleCount() << std::endl;

  return portfolio;
}
