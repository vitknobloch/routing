#pragma once
struct FitnessDiff{
  int fitness;
  int constraints;
  int vehicles;

  FitnessDiff operator +(const FitnessDiff &other) const{
    return FitnessDiff{fitness + other.fitness, constraints + other.constraints, vehicles + other.vehicles};
  }
};