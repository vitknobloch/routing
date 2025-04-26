#include "CVRP/cvrp_pmx_crossover.h"
#include <map>
std::shared_ptr<CvrpIndividual>
CvrpPmxCrossover::crossoverTwo(const std::shared_ptr<CvrpIndividual> parent1,
                               const std::shared_ptr<CvrpIndividual> parent2,
                               uint exchange_start, uint exchange_end) {
  std::shared_ptr<CvrpIndividual> child = std::static_pointer_cast<CvrpIndividual>(parent1->deepcopy());
  child->resetEvaluated();
  auto &data = child->data();
  auto &data_p1 = parent1->data();
  auto &data_p2 = parent2->data();

  std::map<uint, uint> map;

  for(uint i = exchange_start; i <= exchange_end; i++){
    data[i] = data_p2[i];
    map[data_p2[i]] = data_p1[i];
  }

  for(uint i = 0; i < exchange_start; i++){
    while(map.find(data[i]) != map.end())
      data[i] = map[data[i]];
  }

  for(uint i = exchange_end + 1; i < data.size(); i++){
    while(map.find(data[i]) != map.end())
      data[i] = map[data[i]];
  }

  return child;
}

CvrpPmxCrossover::CvrpPmxCrossover() {
  crossover_rate_ = 1.0;
  std::random_device rand;
  gen_ = std::mt19937(rand());
}

void CvrpPmxCrossover::setCrossoverRate(double crossover_rate) {
  crossover_rate_ = crossover_rate;
}

std::shared_ptr<Population>
CvrpPmxCrossover::crossover(const std::shared_ptr<Population> &population,
                            const std::vector<uint> &parents) {
  if(parents.empty())
    return nullptr;
  auto return_pop = population->getEmpty();
  const auto &representative = population->getIndividual(0);
  const auto &representative_ = std::static_pointer_cast<CvrpIndividual>(representative);
  std::uniform_int_distribution<uint> distribution(0, representative_->data().size() - 1);

  for(uint p = 0; p < parents.size() - 1; p++){
    const auto &parent1 = std::static_pointer_cast<CvrpIndividual>(population->getIndividual((int)parents[p]));
    const auto &parent2 = std::static_pointer_cast<CvrpIndividual>(population->getIndividual((int)parents[p+1]));

    uint num1 = distribution(gen_);
    uint num2 = distribution(gen_);
    uint x1 = std::min(num1, num2);
    uint x2 = std::max(num1, num2);

    return_pop->addIndividual(crossoverTwo(parent1, parent2, x1, x2));
    return_pop->addIndividual(crossoverTwo(parent2, parent1, x1, x2));
  }
  return return_pop;
}

double CvrpPmxCrossover::getCrossoverRate() { return crossover_rate_; }
