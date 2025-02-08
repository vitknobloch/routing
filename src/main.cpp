//
// Created by knoblvit on 6.2.25.
//

#include "common/portfolio.h"
#include "common/optal_comms.h"
#include "common/TSP/TSP_serializer.h"

int main(int argc, char* argv[]){
  auto portfolio = std::make_shared<HeuristicPortfolio>();
  auto serializer = std::make_shared<SolutionSerializerTSP>();
  auto optal_comms = std::make_shared<OptalComms>(serializer);

  portfolio->addImprovingHeuristic(optal_comms);
  portfolio->start();

  return 0;
}