#pragma once

#include "individual.h"
#include <memory>


class Neighborhood{
public:
  enum SearchResult {
    IMPROVED /** Better solution found */,
    UNIMPROVED /** No better solution found bur neighborhood not yet fully searched */,
    EXHAUSTED /** No better solution is in the neighborhood */
  };

  virtual SearchResult search(const std::shared_ptr<Individual> &individual) = 0;
  virtual void reset(const std::shared_ptr<Individual> &individual) = 0;
};
