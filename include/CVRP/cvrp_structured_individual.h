//
// Created by knoblvit on 29.3.25.
//

#pragma once

#include "heuristic_framework/individual.h"
#include "common/routing_instance.h"
#include "common/solution.h"

using uint = unsigned int;

struct CvrpRouteSegment{
  uint route_idx;
  uint segment_start_idx;
  uint segment_length;
};

struct CvrpIndividualCustomer{
  uint idx;
  uint time_up_to;
  uint demand_up_to;

  CvrpIndividualCustomer() : idx(0), time_up_to(0), demand_up_to(0) {}
  CvrpIndividualCustomer(uint idx) : idx(idx), time_up_to(0), demand_up_to(0) {}
  CvrpIndividualCustomer(uint idx, uint time_up_to, uint demand_up_to) : idx(idx), time_up_to(time_up_to), demand_up_to(demand_up_to) {}
};

struct CvrpIndividualRoute{
  std::vector<CvrpIndividualCustomer> customers;
  uint demand;
  uint time;

  CvrpIndividualRoute(): customers(), demand(0), time(0) {}
  CvrpIndividualRoute(const CvrpIndividualRoute &cpy) : customers(cpy.customers), demand(cpy.demand), time(cpy.time) {}
  CvrpIndividualRoute(const CvrpIndividualRoute &&mv) : customers(std::move(mv.customers)), demand(std::move(mv.demand)), time(std::move(mv.time)) {}
  CvrpIndividualRoute & operator=(const CvrpIndividualRoute &cpy){
    customers = std::vector<CvrpIndividualCustomer>(cpy.customers);
    time = cpy.time;
    demand = cpy.demand;
    return *this;
  }
  CvrpIndividualRoute & operator=(const CvrpIndividualRoute &&mv){
    customers = std::move(mv.customers);
    time = std::move(mv.time);
    demand = std::move(mv.demand);
    return *this;
  }
};

class CvrpIndividualStructured : public Individual{
private:
  const RoutingInstance *const instance_;
  std::vector<CvrpIndividualRoute> routes_;
  uint total_time_;
  std::vector<double> demand_violation_; // stored as vector for multi-constraint algorithms
  bool is_evaluated_;

  /** Returns a new route where remove segment in to_route is replaced by insert_segment from from_route */
  CvrpIndividualRoute insertSegment(const CvrpIndividualRoute &to_route, const CvrpIndividualRoute &from_route, const CvrpRouteSegment &remove_segment, const CvrpRouteSegment &insert_segment);
  /** Exchanges route on the given index with the new route and performs necessary changes to fitness and demand violation */
  void exchange_route(const uint &route_idx, const CvrpIndividualRoute &new_route);
  /** Returns travel time of the segment (excluding time to move from/to segment) */
  uint getSegmentTime(const CvrpRouteSegment &segment);
  /** Returns demand of the route segment*/
  uint getSegmentDemand(const CvrpRouteSegment &segment);
  /** Returns the time it takes to complete the route after replacing remove segment by insert segment */
  uint getExchangeTime(const CvrpRouteSegment &remove_segment, const CvrpRouteSegment &insert_segment);

public:
  explicit CvrpIndividualStructured(const RoutingInstance * const instance);
  explicit CvrpIndividualStructured(const RoutingInstance * const instance, const std::shared_ptr<Solution> &solution);
  CvrpIndividualStructured(const CvrpIndividualStructured &cpy);
  void initialize() override;
  void resetEvaluated() override;
  bool betterThan(const std::shared_ptr<Individual> &other) override;
  double getFitness() override;
  void calculateConstraints() override;
  void calculateFitness() override;
  const std::vector<double> & getConstraintViolations() override;
  double getTotalConstraintViolation() override;
  bool isEvaluated() override;
  std::shared_ptr<Individual> deepcopy() override;
  void evaluate() override;
  ~CvrpIndividualStructured() override = default;

  const std::vector<CvrpIndividualRoute> &getRoutes();
  /** Reverse the given route segment */
  void perform2optMove(const CvrpRouteSegment &segment);
  /** Exchange the given segments, must be in different routes */
  void performExchangeMove(const CvrpRouteSegment &segment1, const CvrpRouteSegment &segment2);
  /** Move given segment to the target position given by route idx and start_idx insert before start idx, customers.size() for insertion at the end */
  void performRelocateMove(const CvrpRouteSegment &segment_moved, const CvrpRouteSegment &target_pos);
  /** Exchange the ends of the given routes, starting from given start_idx (customers.size() for insertion of other segment at the end) */
  void performCrossMove(const CvrpRouteSegment &segment1, const CvrpRouteSegment &segment2);

  /** Returns true if the 2-opt move will make fitness better
   * (Constant time but still quite demanding) */
  bool test2optMove(const CvrpRouteSegment &segment);
  /** Returns true if the exchange move will make fitness better
   * (Constant time but still quite demanding) */
  bool testExchangeMove(const CvrpRouteSegment &segment1, const CvrpRouteSegment &segment2);
  /** Returns true if the relocate move will make fitness better
   * (Constant time but still quite demanding) */
  bool testRelocateMove(const CvrpRouteSegment &segment_moved, const CvrpRouteSegment &target_pos);
  /** Returns true if the cross move will make fitness better
   * (Constant time but still quite demanding) */
  bool testCrossMove(const CvrpRouteSegment &segment1, const CvrpRouteSegment &segment2);
};