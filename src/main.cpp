#include "CVRP/setup.h"
#include "TSP/setup.h"
#include "VRP-TW/setup.h"
#include "common/portfolio.h"
#include "json.hpp"
#include "common/logger.h"
#include <fstream>
#include <iostream>

using JSON = nlohmann::json;

JSON loadConfig(const char *config_filename)
{
  std::ifstream file(config_filename);
  if (!file)
  {
    std::cerr << "Couldn't open config file " << config_filename << std::endl;
    exit(100);
  }
  JSON config_json;
  try
  {
    file >> config_json;
  }
  catch (const nlohmann::json::exception &ex)
  {
    std::cerr << "Couldn't read JSON object in config file " << config_filename << std::endl;
    exit(100);
  }

  return config_json;
}

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    std::cerr << "Usage: ./Heuristic configFilename instanceFilename logfile [randomSeed]" << std::endl;
  }
  const char *config_filename = argv[1];
  const char *instance_filename = argv[2];
  const char *log_filename = argv[3];

  if (argc > 4)
  {
    errno = 0;
    const int seed = (int)strtol(argv[4], nullptr, 10);
    if (errno != 0)
    {
      std::cerr << "Seed argument present but has invalid value: " << argv[4] << std::endl;
      exit(100);
    }
    srand(seed);
    srandom(seed);
  }

  JSON config_json = loadConfig(config_filename);
  std::shared_ptr<HeuristicPortfolio> portfolio = nullptr;
  if (!config_json.contains("problem"))
  {
    std::cerr << "Problem type not specified in config" << std::endl;
    exit(100);
  }
  else if (config_json["problem"] == "TSP")
  {
    auto setup = SetupTSP();
    portfolio = setup.preparePortfolio(config_json["heuristics"], instance_filename);
  }
  else if (config_json["problem"] == "CVRP")
  {
    auto setup = SetupCVRP();
    portfolio = setup.preparePortfolio(config_json["heuristics"], instance_filename);
  }
  else if (config_json["problem"] == "VRP-TW")
  {
    auto setup = SetupVRPTW();
    portfolio = setup.preparePortfolio(config_json["heuristics"], instance_filename);
  }
  else
  {
    std::cerr << "Unsupported problem type: " << config_json["problem"] << std::endl;
    exit(100);
  }

  if (portfolio == nullptr)
  {
    std::cerr << "Problem opening instance " << instance_filename << std::endl;
  }

  auto logger = std::make_shared<ObjectiveValueLogger>(log_filename);
  portfolio->setLogger(logger);
  portfolio->start();

  return 0;
}