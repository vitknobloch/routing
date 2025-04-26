#pragma once
#include "common/portfolio.h"
#include "json.hpp"

using JSON = nlohmann::json;

class SetupCVRP{
public:
  std::shared_ptr<HeuristicPortfolio> preparePortfolio(const JSON &config, const char* instance_filename);
};
