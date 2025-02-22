//
// Created by knoblvit on 9.2.25.
//
#include "TSP/setup.h"

#include "common/routing_instance.h"
#include "common/optal_comms.h"


std::shared_ptr<HeuristicPortfolio>
SetupTSP::preparePortfolio(const JSON &config, const char *instance_filename) {
  auto portfolio = std::make_shared<HeuristicPortfolio>();
  auto instance = std::make_shared<RoutingInstance>();
  instance->loadTSPlibInstance(instance_filename);

  //Optal communication thread (it has heuristic interface / optal is basically one of the heuristics)
  auto serializer = std::make_shared<SolutionSerializer>();
  auto optalComms = std::make_shared<OptalComms>(serializer);
  portfolio->addImprovingHeuristic(optalComms);


  return portfolio;
}
