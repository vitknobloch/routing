//
// Created by knoblvit on 9.2.25.
//
#include "CVRP//setup.h"

#include "common/routing_instance.h"
#include "common/optal_comms.h"


std::shared_ptr<HeuristicPortfolio>
SetupCVRP::preparePortfolio(const JSON &config, const char *instance_filename) {
  auto portfolio = std::make_shared<HeuristicPortfolio>();
  RoutingInstance instance;
  instance.loadTSPlibInstance(instance_filename);

  //Optal communication thread (it has heuristic interface / optal is basically one of the heuristics)
  auto serializer = std::make_shared<SolutionSerializer>();
  auto optalComms = std::make_shared<OptalComms>(serializer);
  portfolio->addImprovingHeuristic(optalComms);


  return portfolio;
}
