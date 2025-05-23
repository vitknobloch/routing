#include "VRP-TW/vrptw_SA_basic_step.h"
#include <cassert>
VrptwSABasicStep::VrptwSABasicStep() : rand_(), gen_(rand_()), dist_(0.0, 1.0) {
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    try_count[i] = 1;
    success_count[i] = 1;
  }
}

VrptwSABasicStep::neighborhood_options
VrptwSABasicStep::selectNeighborhoodOption() {
  double max_val = 0;
  double vals[neighborhood_options::SIZE];
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    max_val += success_count[i] / (1 + std::log(try_count[i]));
    vals[i] = max_val;
  }

  if(max_val == 0)
    return neighborhood_options::SIZE;

  for(double & val : vals){
    val /= max_val;
  }

  double choice = dist_(gen_);
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    if(choice <= vals[i])
      return static_cast<neighborhood_options>(i);
  }

  assert(false);
  return neighborhood_options::SIZE; // This shouldn't happen
}

void VrptwSABasicStep::success(
    VrptwSABasicStep::neighborhood_options neighborhood) {
  try_count[neighborhood] += 1;
  success_count[neighborhood] += 1;
}

void VrptwSABasicStep::failure(
    VrptwSABasicStep::neighborhood_options neighborhood) {
  try_count[neighborhood] += 1;
}

StepResult
VrptwSABasicStep::step(std::shared_ptr<Individual> &individual,
                       const std::shared_ptr<SABasicSchedule> &schedule) {
  const auto &individual_ = std::static_pointer_cast<VrptwIndividualStructured>(individual);
  neighborhood_options selected = selectNeighborhoodOption();
  if(selected == neighborhood_options::SIZE)
    return SABasicScheduleMemory::UNACCEPTED;

  StepResult result = StepResult::UNACCEPTED;
  if(selected == neighborhood_options::TWO_OPT)
    result = perform2opt(individual_, schedule);
  else if(selected == neighborhood_options::EXCHANGE)
    result = performExchange(individual_, schedule);
  else if(selected == neighborhood_options::RELOCATE)
    result = performRelocate(individual_, schedule);
  else if(selected == neighborhood_options::CROSS)
    result = performCross(individual_, schedule);

  if(result == StepResult::IMPROVED){
    success(selected);
    return result;
  }
  else{
    failure(selected);
    return result;
  }
}

void VrptwSABasicStep::reset(const std::shared_ptr<Individual> &individual){
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    try_count[i] = 1;
    success_count[i] = 1;
  }
}

StepResult VrptwSABasicStep::perform2opt(
    const std::shared_ptr<VrptwIndividualStructured> &individual,
    const std::shared_ptr<SABasicSchedule> &schedule) {
  uint route_idx = randomSelectRoute(individual, 2);
  auto segment = randomSelectSegment(individual, route_idx, 2, false);
  assert(segment.route_idx < individual->getRoutes().size());

  if(individual->test2optMove(segment)){
    individual->perform2optMove(segment);
    return StepResult::IMPROVED;
  }
  else if(schedule->shouldAcceptSolution()){
    individual->perform2optMove(segment);
    return StepResult::ACCEPTED;
  }
  return StepResult::UNACCEPTED;
}

StepResult VrptwSABasicStep::performExchange(
    const std::shared_ptr<VrptwIndividualStructured> &individual,
    const std::shared_ptr<SABasicSchedule> &schedule) {
  uint route1_idx = randomSelectRoute(individual, 1);
  uint route2_idx = randomSelectRoute(individual, 1, (int)route1_idx);
  auto segment1 = randomSelectSegment(individual, route1_idx, 1, false);
  auto segment2 = randomSelectSegment(individual, route2_idx, 1, false);
  segment1.segment_length = 1;
  segment2.segment_length = 1;
  assert(segment1.route_idx < individual->getRoutes().size() && segment2.route_idx < individual->getRoutes().size());


  if(individual->testExchangeMove(segment1, segment2)){
    individual->performExchangeMove(segment1, segment2);
    return StepResult::IMPROVED;
  }
  else if(schedule->shouldAcceptSolution()){
    individual->performExchangeMove(segment1, segment2);
    return StepResult::ACCEPTED;
  }
  return StepResult::UNACCEPTED;
}

StepResult VrptwSABasicStep::performRelocate(
    const std::shared_ptr<VrptwIndividualStructured> &individual,
    const std::shared_ptr<SABasicSchedule> &schedule) {
  uint route1_idx = randomSelectRoute(individual, 1);
  uint route2_idx = randomSelectRoute(individual, 0, (int)route1_idx);
  auto segment_from = randomSelectSegment(individual, route1_idx, 1, false);
  auto segment_to = randomSelectSegment(individual, route2_idx, 0, true);
  segment_from.segment_length = 1;
  segment_to.segment_length = 0;
  assert(segment_to.route_idx < individual->getRoutes().size() && segment_from.route_idx < individual->getRoutes().size());


  if(individual->testRelocateMove(segment_from, segment_to)){
    individual->performRelocateMove(segment_from, segment_to);
    return StepResult::IMPROVED;
  }
  else if(schedule->shouldAcceptSolution()){
    individual->performRelocateMove(segment_from, segment_to);
    return StepResult::ACCEPTED;
  }
  return StepResult::UNACCEPTED;
}

StepResult VrptwSABasicStep::performCross(
    const std::shared_ptr<VrptwIndividualStructured> &individual,
    const std::shared_ptr<SABasicSchedule> &schedule) {
  uint route1_idx = randomSelectRoute(individual, 1);
  uint route2_idx = randomSelectRoute(individual, 1, (int)route1_idx);
  auto segment1 = randomSelectSegment(individual, route1_idx, 1, false);
  auto segment2 = randomSelectSegment(individual, route2_idx, 0, true);
  assert(segment1.route_idx < individual->getRoutes().size() && segment2.route_idx < individual->getRoutes().size());

  if(individual->testCrossMove(segment1, segment2)){
    individual->performCrossMove(segment1, segment2);
    return StepResult::IMPROVED;
  }
  else if(schedule->shouldAcceptSolution()){
    individual->performCrossMove(segment1, segment2);
    return StepResult::ACCEPTED;
  }
  return StepResult::UNACCEPTED;
}

uint VrptwSABasicStep::randomSelectRoute(
    const std::shared_ptr<VrptwIndividualStructured> &individual,
    uint min_length, int exclude_route) {
  const auto &routes = individual->getRoutes();
  std::uniform_int_distribution<uint> route_dist(0, routes.size()-1);
  uint route_idx = route_dist(gen_);
  uint original_route_idx = route_idx;
  do{
    if(routes[route_idx].customers.size() >= min_length && (int)route_idx != exclude_route)
      return route_idx;
    route_idx = (route_idx + 1) % routes.size();
  }while(route_idx != original_route_idx);

  return routes.size();
}

VrptwRouteSegment VrptwSABasicStep::randomSelectSegment(
    const std::shared_ptr<VrptwIndividualStructured> &individual, uint route_idx,
    uint min_length, bool include_end) {
  VrptwRouteSegment segment{};
  segment.route_idx = route_idx;
  const auto &routes = individual->getRoutes();
  if(route_idx >= routes.size())
    return segment;
  const auto &route = routes[route_idx];

  assert(!include_end || min_length == 0);
  uint max_start_idx = route.customers.size() - min_length;
  if(!include_end && min_length == 0)
    max_start_idx--;

  if (max_start_idx == 0) {
    segment.segment_start_idx = 0;
  } else {
    std::uniform_int_distribution<uint> start_dist(0, max_start_idx);
    segment.segment_start_idx = start_dist(gen_);
  }

  const uint max_length = (uint)route.customers.size() - segment.segment_start_idx;
  if (max_length == min_length) {
    segment.segment_length = min_length;
  } else {
    std::uniform_int_distribution<uint> length_dist(min_length, max_length);
    segment.segment_length = length_dist(gen_);
  }

  return segment;
}
