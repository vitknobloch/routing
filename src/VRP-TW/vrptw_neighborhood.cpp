#include "VRP-TW/vrptw_neighborhood.h"
#include <cassert>
#include <iostream>

VrptwNeighborhood::VrptwNeighborhood() : rand_(), gen_(rand_()), dist_(0.0, 1.0) {
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    exhausted_[i] = false;
    try_count[i] = 1;
    success_count[i] = 1;
  }
}

VrptwNeighborhood::neighborhood_options
VrptwNeighborhood::selectNeighborhoodOption() {
  double max_val = 0;
  double vals[neighborhood_options::SIZE];
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    if(!exhausted_[i])
      max_val += success_count[i] / try_count[i];
    vals[i] = max_val;
  }

  if(max_val == 0)
    return neighborhood_options::SIZE;
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    vals[i] /= max_val;
  }

  double choice = dist_(gen_);
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    if(choice <= vals[i])
      return static_cast<neighborhood_options>(i);
  }

  assert(false);
  return neighborhood_options::SIZE; // This shouldn't happen
}

Neighborhood::SearchResult
VrptwNeighborhood::search(const std::shared_ptr<Individual> &individual) {
  const auto &individual_ = std::static_pointer_cast<VrptwIndividualStructured>(individual);
  neighborhood_options selected = selectNeighborhoodOption();
  if(selected == neighborhood_options::SIZE)
    return Neighborhood::SearchResult::EXHAUSTED;

  bool result = false;
  if(selected == neighborhood_options::TWO_OPT)
    result = perform2opt(individual_);
  else if(selected == neighborhood_options::EXCHANGE)
    result = performExchange(individual_);
  else if(selected == neighborhood_options::RELOCATE)
    result = performRelocate(individual_);
  else if(selected == neighborhood_options::CROSS)
    result = performCross(individual_);

  if(result){
    success(selected);
    return Neighborhood::SearchResult::IMPROVED;
  }
  else{
    failure(selected);
    return Neighborhood::SearchResult::UNIMPROVED;
  }
}

void VrptwNeighborhood::success(
    VrptwNeighborhood::neighborhood_options neighborhood) {
  for(int i = 0; i < neighborhood_options::SIZE; i++){
    exhausted_[i] = false;
  }
  try_count[neighborhood] += 1;
  success_count[neighborhood] += 1;
}

void VrptwNeighborhood::failure(
    VrptwNeighborhood::neighborhood_options neighborhood) {
  exhausted_[neighborhood] = true;
  try_count[neighborhood] += 1;
}

void VrptwNeighborhood::reset(const std::shared_ptr<Individual> &individual) {
  for(int i = 0; i < neighborhood_options::SIZE; i++)
    exhausted_[i] = false;
}
bool VrptwNeighborhood::perform2opt(
    const std::shared_ptr<VrptwIndividualStructured> &individual) {
  bool performed = false;

  const auto &routes = individual->getRoutes();
  VrptwRouteSegment segment{};
  for(uint r = 0; r < routes.size(); r++){
    segment.route_idx = r;
    //std::cerr << "2-opt:      " << r << std::endl;
    for(int i = 0; i < (int)routes[r].customers.size() - 1; i++){
      segment.segment_start_idx = i;
      const uint max_length = routes[r].customers.size() - i;
      for(uint l = 2; l <= max_length; l++){
        segment.segment_length = l;
        if(individual->test2optMove(segment)){
          individual->perform2optMove(segment);
          performed = true;
        }
      }
    }
  }
  return performed;
}

bool VrptwNeighborhood::performExchange(
    const std::shared_ptr<VrptwIndividualStructured> &individual) {
  bool performed = false;
  const auto &routes = individual->getRoutes();
  VrptwRouteSegment segment1{}, segment2{};
  segment1.segment_length = 1;
  segment2.segment_length = 1;

  // Each pair of routes
  for(uint r1 = 0; r1 < routes.size() - 1; r1++){
    segment1.route_idx = r1;
    for(uint r2 = r1+1; r2 < routes.size(); r2++){
      segment2.route_idx = r2;
      //std::cerr << "exchange:   " << r1 << " " << r2 << std::endl;
      //Each pair of customers
      for(uint c1 = 0; c1 < routes[r1].customers.size(); c1++){
        segment1.segment_start_idx = c1;
        for(uint c2 = 0; c2 < routes[r2].customers.size(); c2++){
          segment2.segment_start_idx = c2;
          //try the exchange
          if(individual->testExchangeMove(segment1, segment2)){
            individual->performExchangeMove(segment1, segment2);
            performed = true;
          }
        }
      }
    }
  }

  return performed;
}

bool VrptwNeighborhood::performRelocate(
    const std::shared_ptr<VrptwIndividualStructured> &individual) {
  bool performed = false;
  const auto &routes = individual->getRoutes();
  VrptwRouteSegment segment_move{}, target_pos{};
  segment_move.segment_length = 1;
  target_pos.segment_length = 0;

  for(uint r_from = 0; r_from < routes.size(); r_from++){
    segment_move.route_idx = r_from;
    for(uint r_to = 0; r_to < routes.size(); r_to++){
      if(r_from == r_to) // Relocation within route is not possible with this operator
        continue;
      target_pos.route_idx = r_to;
      //std::cerr << "relocate:   " << r_from << " " << r_to << std::endl;

      for(uint c1 = 0; c1 < routes[r_from].customers.size(); c1++){
        segment_move.segment_start_idx = c1;
        bool performed_for_customer = false;
        for(uint c2 = 0; c2 <= routes[r_to].customers.size(); c2++){
          target_pos.segment_start_idx = c2;
          if(individual->testRelocateMove(segment_move, target_pos)){
            individual->performRelocateMove(segment_move, target_pos);
            performed = true;
            performed_for_customer = true;
            break; //c1 customer will change
          }
        }
        /*if(performed_for_customer)
          c1--; // the next customer will shift if the relocation is performed*/
      }
    }
  }

  return performed;
}

bool VrptwNeighborhood::performCross(
    const std::shared_ptr<VrptwIndividualStructured> &individual) {
  bool performed = false;
  const auto &routes = individual->getRoutes();
  VrptwRouteSegment segment1{}, segment2{};
  segment1.segment_length = 0;
  segment2.segment_length = 0;

  // Each pair of routes
  for(uint r1 = 0; r1 < routes.size() - 1; r1++){
    segment1.route_idx = r1;
    for(uint r2 = r1 + 1; r2 < routes.size(); r2++){
      segment2.route_idx = r2;
      //std::cerr << "cross:      " << r1 << " " << r2 << std::endl;

      //Each pair of customers
      for(uint c1 = 0; c1 <= routes[r1].customers.size(); c1++){
        segment1.segment_start_idx = c1;
        bool performed_for_customer = false;
        for(uint c2 = 0; c2 <= routes[r2].customers.size(); c2++){
          segment2.segment_start_idx = c2;
          //try the exchange
          if(individual->testCrossMove(segment1, segment2)){
            individual->performCrossMove(segment1, segment2);
            performed = true;
            performed_for_customer = true;
            break; //c1 customer will change
          }
        }
        /*if(performed_for_customer)
          c1--; // c1 customer changed, reevaluate same index */
      }
    }
  }

  return performed;
}
