#pragma once
#include <vector>
#include <memory>

class Individual{
public:
  virtual bool isEvaluated() = 0;
  virtual void resetEvaluated() = 0;
  virtual void evaluate() = 0;
  virtual void calculateFitness() = 0;
  virtual void calculateConstraints() = 0;
  /** Basic initialization to correct individual */
  virtual void initialize() = 0;
  /** Smart randomized initialization with possible basic optimizations */
  virtual void smartInitialize() = 0;
  virtual double getFitness() = 0;
  virtual double getTotalConstraintViolation() = 0;
  virtual const std::vector<double> &getConstraintViolations() = 0;
  virtual bool betterThan(const std::shared_ptr<Individual> &other) = 0;
  virtual std::shared_ptr<Individual> deepcopy() = 0;
  virtual ~Individual() = default;
};