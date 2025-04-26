#pragma once
#include "common/routing_instance.h"
#include "vrptw_individual.h"
#include "heuristic_framework/mutation.h"
#include <random>

class VrptwMutationRandom : public Mutation {
private:
  std::random_device rand_;
  std::mt19937 gen_;
  std::shared_ptr<RoutingInstance> instance_;
  double mutation_rate_;

private:
  bool mutate2optInsideRoute(const std::shared_ptr<VrptwIndividual> &individual);
  bool mutateExchange(const std::shared_ptr<VrptwIndividual> &individual);
  bool mutateShift(const std::shared_ptr<VrptwIndividual> &individual);

  inline bool isCustomer(const uint &node);
  inline bool isDepot(const uint &node);

public:
  explicit VrptwMutationRandom(const std::shared_ptr<RoutingInstance> &instance);
  bool isInPlace() override;
  bool mutate(const std::shared_ptr<Individual> &individual) override;
  double getMutationRate() override;
  void setMutationRate(double mutation_rate);
};