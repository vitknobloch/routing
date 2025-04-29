#pragma once
#include "routing_instance.h"
#include <functional>

std::function<double(double)> getExpSchedule(double initial_temperature, double final_temperature, double duration);
std::function<double(double)> getLinearSchedule(double initial_temperature, double final_temperature, double duration);

double getAverageEdgeLength(const RoutingInstance &instance);
double getTemperatureByTargetAcceptanceRate(const double &length_diff, double target_acceptance_rate);

/* Get function that calculates convertion rate so that per_units difference in
 * a measure (vehicle_count / constraint violation) is equal to fitness_diff in acceptance rate.
 */
std::function<double(double)> getEquivalentPunishmentFunction(double fitness_diff, double per_units);