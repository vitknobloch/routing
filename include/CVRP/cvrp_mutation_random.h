//
// Created by knoblvit on 24.2.25.
//

#pragma once
#include "common/routing_instance.h"
#include "cvrp_individual.h"
#include "heuristic_framework/mutation.h"
#include <random>

class CvrpMutationRandom : public Mutation {
private:
  std::random_device rand;
  std::mt19937 gen;
  std::shared_ptr<RoutingInstance> instance_;
  double mutation_rate_;

  void getRouteStartAndLength(const std::vector<uint> &data, uint search_from, uint *start, uint *length);
  void mutate2optInsideRoute(std::shared_ptr<CvrpIndividual> &individual, uint start_route, uint length_route);
  int wrapWithinSpan(int begin, int length, int initial, int shift);
  void mutateKick(std::shared_ptr<CvrpIndividual> &individual);
  void mutateExchange(std::shared_ptr<CvrpIndividual> &individual);
  void mutateExchangeDemandViolated(std::shared_ptr<CvrpIndividual> &individual);
  uint selectIdxToKick(std::shared_ptr<CvrpIndividual> &individual);
  uint selectIdxToKickDemandViolated(std::shared_ptr<CvrpIndividual> &individual, uint start_idx);
  int selectIdxToExchange(std::shared_ptr<CvrpIndividual> &individual, uint kicked_idx);
  int selectInsertIndex(std::shared_ptr<CvrpIndividual> &individual, uint kicked_idx);
  int selectIdxToExchangeDemandViolated(std::shared_ptr<CvrpIndividual> &sharedPtr, uint kicked_idx);
  bool mutate2opt(std::shared_ptr<CvrpIndividual> &individual);

  inline bool isCustomer(const uint &node);
  inline bool isDepot(const uint &node);

public:
  CvrpMutationRandom(const std::shared_ptr<RoutingInstance> &instance);
  bool isInPlace() override;
  bool mutate(const std::shared_ptr<Individual> &individual) override;
  double getMutationRate() override;
  void setMutationRate(double mutation_rate);

};