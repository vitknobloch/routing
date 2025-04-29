#include "common/SA_schedule_functions.h"
#include <cmath>
#include <cstdlib>
std::function<double(double)> getExpSchedule(double initial_temperature,
                                               double final_temperature,
                                               double duration) {

  const double a = initial_temperature;
  const double b = std::log(final_temperature / initial_temperature) / duration;

  return [a, b, final_temperature](double t) { return std::max(a * std::exp(b * t), final_temperature); };
}

std::function<double(double)> getLinearSchedule(double initial_temperature,
                                                  double final_temperature,
                                                  double duration) {
  const double a = (final_temperature - initial_temperature) / duration;
  const double b = initial_temperature;
  return [a, b, final_temperature](double t) { return std::max(a * t + b, final_temperature); };
}

double getAverageEdgeLength(const RoutingInstance &instance) {
  const uint * const matrix = instance.getMatrix().get();
  const uint node_count = instance.getNodesCount();
  double sum = 0.0;
  for(uint i = 0; i < node_count; i++)
    for(uint j = i + 1; j < node_count; j++)
      sum += matrix[i * node_count + j];
  return sum / (node_count * (node_count - 1) / 2);
}

double getTemperatureByTargetAcceptanceRate(const double &length_diff,
                                          double target_acceptance_rate) {
  return - length_diff / std::log(target_acceptance_rate);
}

std::function<double(double)>
getEquivalentPunishmentFunction(double fitness_diff, double per_units) {
  return [fitness_diff, per_units](double t) { return fitness_diff / per_units; };
}
