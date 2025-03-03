//
// Created by knoblvit on 20.2.25.
//

#pragma once

#include "individual.h"
#include <memory>

class Mutation{
public:
  virtual bool isInPlace() = 0; //returns true if the mutation only happens when the new individual is better
  virtual bool mutate(const std::shared_ptr<Individual> &individual) = 0; // return true if returned individual is evaluated
  virtual double getMutationRate() = 0;
};
