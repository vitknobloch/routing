//
// Created by knoblvit on 9.2.25.
//
#include "CVRP//setup.h"

#include "CVRP/cvrp_local_search.h"
#include "CVRP/cvrp_mutation_random.h"
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

  auto mutation = std::make_shared<CvrpMutationRandom>(instance);
  auto localSearch = std::make_shared<CvrpLocalSearch>(instance, mutation);
  portfolio->addImprovingHeuristic(localSearch);

  return portfolio;
}
