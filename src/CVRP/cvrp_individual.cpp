//
// Created by knoblvit on 24.2.25.
//
#include "CVRP/cvrp_individual.h"
#include <limits>

CvrpIndividual::CvrpIndividual(const RoutingInstance *const instance) :
 instance_(instance), matrix_(instance->getMatrix().get()), data_(), is_evaluated_(false),
  fitness_(std::numeric_limits<double>::max()), capacity_constraint_violation_(0){}

CvrpIndividual::CvrpIndividual(const CvrpIndividual &other) :
instance_(other.instance_), matrix_(other.matrix_), data_(other.data_), is_evaluated_(other.is_evaluated_),
  fitness_(other.fitness_), capacity_constraint_violation_(other.capacity_constraint_violation_){}

void CvrpIndividual::initialize() {
  data_.reserve(instance_->getNodesCount() + instance_->getVehicleCount());
  for(int i = 0; i < instance_->getNodesCount() + instance_->getVehicleCount(); i++){
    data_.push_back(i);
  }
}

std::vector<uint> &CvrpIndividual::data() { return data_; }

void CvrpIndividual::resetEvaluated() {
  is_evaluated_ = false;
}

void CvrpIndividual::setFitness(double fitness) {
  fitness_ = fitness;
  is_evaluated_ = true;
}

bool CvrpIndividual::betterThan(const std::shared_ptr<Individual> &other) {
  auto individual_ = std::static_pointer_cast<CvrpIndividual>(other);
  if(capacity_constraint_violation_ < other->getTotalConstraintViolation())
    return true;
  else if(capacity_constraint_violation_ == other->getTotalConstraintViolation() &&
           fitness_ < other->getFitness())
    return true;
  return false;
}

void CvrpIndividual::calculateFitness() {
  fitness_ = 0;
  for(uint i = 0; i < data_.size() - 1; i++){
    fitness_ += instance_->getDistance(data_[i], data_[i+1]);
  }
  fitness_ += instance_->getDistance(data_[data_.size() - 1], data_[0]);
}

std::shared_ptr<Individual> CvrpIndividual::deepcopy() {
  return std::make_shared<CvrpIndividual>(*this);
}
double CvrpIndividual::getFitness() {
  if(!is_evaluated_)
    evaluate();
  return fitness_;
}

bool CvrpIndividual::isEvaluated() {
  return is_evaluated_;
}

void CvrpIndividual::calculateConstraints() {
  uint first_vehicle_start = data_.size();
  uint i = 0;
  uint demand_running_sum = 0;
  uint demand_violation = 0;
  while(i != first_vehicle_start && i < data_.size()){
    if(data_[i] == 0 || data_[i] >= (uint)instance_->getNodesCount()){
      if(first_vehicle_start == data_.size()){
        first_vehicle_start = i;
      }
      demand_violation += std::max(demand_running_sum - instance_->getVehicleCapacity(), 0u);
      demand_running_sum = 0;
    }
    i = (i + 1) % data_.size();
  }
  capacity_constraint_violation_ = demand_violation;
}
const std::vector<double> &CvrpIndividual::getConstraintViolations() {
  return {capacity_constraint_violation_};
}
double CvrpIndividual::getTotalConstraintViolation() {
  return capacity_constraint_violation_;
}

void CvrpIndividual::evaluate() {
  if(!is_evaluated_){
    calculateFitness();
    calculateConstraints();
  }
  is_evaluated_ = true;
}
