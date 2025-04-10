//
// Created by knoblvit on 1.4.25.
//

#pragma once

#include "heuristic_framework/individual.h"
#include "common/routing_instance.h"
#include "common/solution.h"

using uint = unsigned int;

struct VrptwRouteSegment{
  uint route_idx;
  uint segment_start_idx;
  uint segment_length;
};

struct VrptwIndividualCustomer{
  uint idx;
  uint travel_time_up_to;
  uint time_up_to;
  uint demand_up_to;

  VrptwIndividualCustomer() : idx(0), travel_time_up_to(0), time_up_to(0), demand_up_to(0) {}
  explicit VrptwIndividualCustomer(uint idx) : idx(idx), travel_time_up_to(0), time_up_to(0), demand_up_to(0) {}
  VrptwIndividualCustomer(uint idx, uint travel_time_up_to, uint time_up_to, uint demand_up_to) : idx(idx), travel_time_up_to(travel_time_up_to), time_up_to(time_up_to), demand_up_to(demand_up_to) {}
};

struct VrptwIndividualRoute{
  std::vector<VrptwIndividualCustomer> customers;
  uint demand;
  uint time;
  uint travel_time;
  uint time_violation;

  VrptwIndividualRoute(): customers(), demand(0), time(0), travel_time(0), time_violation(0) {}
  VrptwIndividualRoute(const VrptwIndividualRoute &cpy) : customers(cpy.customers), demand(cpy.demand), time(cpy.time), travel_time(cpy.travel_time), time_violation(cpy.time_violation) {}
  VrptwIndividualRoute(VrptwIndividualRoute &&mv)  noexcept : customers(std::move(mv.customers)), demand(mv.demand), time(mv.time), travel_time(mv.travel_time), time_violation(mv.time_violation) {}
  VrptwIndividualRoute & operator=(const VrptwIndividualRoute &cpy){
    customers = std::vector<VrptwIndividualCustomer>(cpy.customers);
    time = cpy.time;
    travel_time = cpy.travel_time;
    demand = cpy.demand;
    time_violation = cpy.time_violation;
    return *this;
  }
  VrptwIndividualRoute & operator=(VrptwIndividualRoute &&mv) noexcept {
    customers = std::move(mv.customers);
    time = mv.time;
    travel_time = mv.travel_time;
    demand = mv.demand;
    time_violation = mv.time_violation;
    return *this;
  }
};

class VrptwIndividualStructured : public Individual{
private:
  const RoutingInstance *const instance_;
  std::vector<VrptwIndividualRoute> routes_;
  uint total_time_;
  uint total_travel_time_;
  uint vehicles_used_;
  std::vector<double> violations_; //0 - demand, 1 - time
  bool is_evaluated_;

  /** Returns a new route where remove segment in to_route is replaced by insert_segment from from_route */
  VrptwIndividualRoute insertSegment(const VrptwIndividualRoute &to_route, const VrptwIndividualRoute &from_route, const VrptwRouteSegment &remove_segment, const VrptwRouteSegment &insert_segment);
  /** Exchanges route on the given index with the new route and performs necessary changes to fitness and demand violation */
  void exchange_route(const uint &route_idx, const VrptwIndividualRoute &new_route);
  /** Returns travel time of the segment (excluding time to move from/to segment) */
  uint getSegmentTravelTime(const VrptwRouteSegment &segment);
  /** Returns demand of the route segment*/
  uint getSegmentDemand(const VrptwRouteSegment &segment);
  /** Returns the time it takes to complete the route after replacing remove segment by insert segment */
  uint getExchangeTravelTime(const VrptwRouteSegment &remove_segment, const VrptwRouteSegment &insert_segment);

  bool exchangeViolatesTimeConstraints(const VrptwRouteSegment &remove_segment, const VrptwRouteSegment &insert_segment);
  int exchangeTimeViolationChange(const VrptwRouteSegment &remove_segment, const VrptwRouteSegment &insert_segment);

  bool test2optMoveNoViolations(const VrptwRouteSegment &segment);
  bool test2optMoveViolation(const VrptwRouteSegment &segment);

  bool testExchangeMoveNoViolation(const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2);
  bool testExchangeMoveViolation(const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2);

  uint getArrivalTime(const VrptwIndividualRoute &route, const uint &customer_idx);
  uint getEndTime(const VrptwIndividualRoute &route, const int &customer_idx);

  void evaluateRoute(uint route_idx);
  inline double& capacityViolation() {return violations_[0];}
  inline double& timeViolation() {return violations_[1];}

  bool assertEvaluation(const VrptwIndividualRoute &route);

public:
  explicit VrptwIndividualStructured(const RoutingInstance * const instance);
  explicit VrptwIndividualStructured(const RoutingInstance * const instance, const std::shared_ptr<Solution> &solution);
  VrptwIndividualStructured(const VrptwIndividualStructured &cpy);
  std::shared_ptr<Solution> convertSolution();
  void initialize() override;
  void smartInitialize() override;
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
  ~VrptwIndividualStructured() override = default;

  const std::vector<VrptwIndividualRoute> &getRoutes();
  /** Reverse the given route segment */
  void perform2optMove(const VrptwRouteSegment &segment);
  /** Exchange the given segments, must be in different routes */
  void performExchangeMove(const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2);
  /** Move given segment to the target position given by route idx and start_idx insert before start idx, customers.size() for insertion at the end */
  void performRelocateMove(const VrptwRouteSegment &segment_moved, const VrptwRouteSegment &target_pos);
  /** Exchange the ends of the given routes, starting from given start_idx (customers.size() for insertion of other segment at the end) */
  void performCrossMove(const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2);

  /** Returns true if the 2-opt move will make fitness better */
  bool test2optMove(const VrptwRouteSegment &segment);
  /** Returns true if the exchange move will make fitness better */
  bool testExchangeMove(const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2);
  /** Returns true if the relocate move will make fitness better */
  bool testRelocateMove(const VrptwRouteSegment &segment_moved, const VrptwRouteSegment &target_pos);
  /** Returns true if the cross move will make fitness better */
  bool testCrossMove(const VrptwRouteSegment &segment1, const VrptwRouteSegment &segment2);
};