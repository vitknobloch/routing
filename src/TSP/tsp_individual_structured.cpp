#include "TSP/tsp_individual_structured.h"
#include <cassert>
#include <limits>
#include <random>

TspIndividualStructured::TspIndividualStructured(
    const RoutingInstance *const instance) : instance_(instance), data_(), is_evaluated_(false), total_time_(0) {

}

TspIndividualStructured::TspIndividualStructured(
    const RoutingInstance *const instance,
    const std::shared_ptr<Solution> &solution) : TspIndividualStructured(instance){
  data_.clear();
  data_.reserve(instance->getNodesCount() - 1);
  for(const auto &node : solution->routes.begin()->route_nodes){
    if(node.idx == 0)
      continue;
    data_.push_back(node.idx);
  }
  evaluate();
  is_evaluated_ = true;
}

TspIndividualStructured::TspIndividualStructured(
    const TspIndividualStructured &cpy) : instance_(cpy.instance_), data_(cpy.data_), is_evaluated_(cpy.is_evaluated_), total_time_(cpy.total_time_){

}

std::shared_ptr<Solution> TspIndividualStructured::convertSolution() {
  auto solution = std::make_shared<Solution>();
  solution->feasible = true;
  solution->objective = total_time_;
  solution->used_vehicles = 1;
  solution->travel_time_sum = total_time_;

  uint time = 1;
  solution->routes.emplace_back();
  auto &route = solution->routes.back();
  route.route_nodes.emplace_back(0, 0, 1);
  uint prev_node = 0;
  for(const auto &node : data_){
    time += instance_->getDistance(prev_node, node);
    route.route_nodes.emplace_back(node, time, time + 1);
    time++;
    prev_node = node;
  }
  time += instance_->getDistance(prev_node, 0);
  route.route_nodes.emplace_back(0, time, time + 1);
  route.demand = 0;
  route.travel_time = solution->travel_time_sum;
  route.end_time = time;
  solution->end_time_sum = time;

  return solution;
}
void TspIndividualStructured::initialize() {
  data_.clear();
  data_.reserve(instance_->getNodesCount() - 1);
  for(int i = 1; i < instance_->getNodesCount(); i++){
    data_.push_back(i);
  }
  is_evaluated_ = false;
}

void TspIndividualStructured::smartInitialize() {
  data_.clear();
  data_.reserve(instance_->getNodesCount() - 1);
  std::vector<bool> used(instance_->getNodesCount(), false);

  std::random_device rand;
  std::mt19937 gen(rand());
  std::uniform_int_distribution<uint> dist(0, instance_->getNodesCount() - 1);

  std::vector<uint> nodes;
  nodes.reserve(instance_->getNodesCount());
  uint prev_node= dist(gen);
  nodes.push_back(prev_node);
  used[prev_node] = true;
  uint zero_idx = 0;

  for(int i = 1; i < instance_->getNodesCount(); i++){
    uint best_distance = std::numeric_limits<uint>::max();
    uint nearest_neighbor = 0;
    for(int j = 0; j < instance_->getNodesCount(); j++){
      if(used[j])
        continue;
      const uint distance = instance_->getDistance(prev_node, j);
      if(distance < best_distance){
        best_distance = distance;
        nearest_neighbor = j;
      }
    }
    assert(used[nearest_neighbor] == false);
    used[nearest_neighbor] = true;
    nodes.push_back(nearest_neighbor);
    if(nearest_neighbor == 0){
      zero_idx = i;
    }
  }
  is_evaluated_ = false;
  for(int i = 1; i < instance_->getNodesCount(); i++){
    data_.push_back(nodes[(zero_idx + i) % nodes.size()]);
  }
  evaluate();
}
void TspIndividualStructured::resetEvaluated() {
  is_evaluated_ = false;
}
bool TspIndividualStructured::betterThan(
    const std::shared_ptr<Individual> &other) {
  if(!other)
    return true;
  auto individual_ = std::static_pointer_cast<TspIndividualStructured>(other);
  return total_time_ < individual_->total_time_;
}
double TspIndividualStructured::getFitness() { return total_time_;}
void TspIndividualStructured::calculateConstraints() {}
void TspIndividualStructured::calculateFitness() {
  total_time_ = 0;
  uint prev_node = 0;
  for(unsigned int node : data_){
    total_time_ += instance_->getDistance(prev_node, node);
    prev_node = node;
  }
  total_time_ += instance_->getDistance(prev_node, 0);
}
const std::vector<double> &TspIndividualStructured::getConstraintViolations() {
  return {};
}
double TspIndividualStructured::getTotalConstraintViolation() { return 0; }
bool TspIndividualStructured::isEvaluated() { return is_evaluated_;}
std::shared_ptr<Individual> TspIndividualStructured::deepcopy() {
  return std::make_shared<TspIndividualStructured>(*this);
}
void TspIndividualStructured::evaluate() {
  if(!is_evaluated_)
    calculateFitness();
  is_evaluated_ = true;
}
const std::vector<uint> &TspIndividualStructured::getData() {
  return data_;
}
void TspIndividualStructured::perform2optMove(
    const TspIndividualSegment &segment) {
  assert(segment.start_idx < segment.end_idx);
  assert(segment.start_idx < data_.size());
  assert(segment.end_idx < data_.size());

  uint high = segment.end_idx;
  uint low = segment.start_idx;
  const uint prev_node = low > 0 ? data_[low - 1] : 0;
  const uint next_node = high < data_.size() - 1 ? data_[high + 1] : 0;
  const uint prev_distance = instance_->getDistance(prev_node, data_[low]) + instance_->getDistance(data_[high], next_node);
  const uint new_distance = instance_->getDistance(prev_node, data_[high]) + instance_->getDistance(data_[low], next_node);
  total_time_ += new_distance;
  total_time_ -= prev_distance;
  while(low < high){
    uint tmp = data_[low];
    data_[low] = data_[high];
    data_[high] = tmp;
    low++;
    high--;
  }
}
bool TspIndividualStructured::test2optMove(
    const TspIndividualSegment &segment) {
  assert(segment.start_idx < segment.end_idx);
  assert(segment.start_idx < data_.size());
  assert(segment.end_idx < data_.size());

  const uint high = segment.end_idx;
  const uint low = segment.start_idx;
  const uint prev_node = low > 0 ? data_[low - 1] : 0;
  const uint next_node = high < data_.size() - 1 ? data_[high + 1] : 0;
  const uint prev_distance = instance_->getDistance(prev_node, data_[low]) + instance_->getDistance(data_[high], next_node);
  const uint new_distance = instance_->getDistance(prev_node, data_[high]) + instance_->getDistance(data_[low], next_node);
  return new_distance < prev_distance;
}

void TspIndividualStructured::performDoubleBridgeMove(
    const TspIndividualSegment &segment1,
    const TspIndividualSegment &segment2) {
  assert(segment1.start_idx < segment1.end_idx);
  assert(segment1.end_idx < segment2.start_idx);
  assert(segment2.start_idx < segment2.end_idx);
  assert(segment1.start_idx < data_.size());
  assert(segment1.end_idx < data_.size());
  assert(segment2.start_idx < data_.size());
  assert(segment2.end_idx < data_.size());

  std::vector<uint> nodes(data_);
  uint idx = 0;
  for(int i = 0; i < segment1.start_idx; i++){
    data_[idx++] = nodes[i];
  }
  for(int i = segment2.start_idx; i <= segment2.end_idx; i++){
    data_[idx++] = nodes[i];
  }
  for(int i = segment1.start_idx; i <= segment1.end_idx; i++){
    data_[idx++] = nodes[i];
  }
  for(int i = segment1.end_idx + 1; i < segment2.start_idx; i++){
    data_[idx++] = nodes[i];
  }
  for(int i = segment2.end_idx + 1; i < nodes.size(); i++){
    data_[idx++] = nodes[i];
  }
  resetEvaluated();
  evaluate();
}

TspIndividualStructured::TspIndividualStructured(
    const TspIndividualStructured &cpy, const std::vector<uint> &flat_data) : TspIndividualStructured(cpy.instance_){
  data_.clear();
  data_.reserve(instance_->getNodesCount() - 1);
  uint zero_idx = 0;
  while(flat_data[zero_idx] != 0)
    zero_idx++;

  for(int i = 1; i < instance_->getNodesCount(); i++){
    data_.push_back(flat_data[(zero_idx + i) % flat_data.size()]);
  }
  is_evaluated_ = false;
  evaluate();
}

std::vector<uint> TspIndividualStructured::flatten() {
  std::vector<uint> flat_data(1, 0);
  flat_data.insert(flat_data.end(), data_.begin(), data_.end());
  return flat_data;
}

void TspIndividualStructured::performSwapMove(uint idx1, uint idx2) {
  assert(std::abs((int)idx1 - (int)idx2) > 1);
  assert(idx1 < data_.size() && idx2 < data_.size());
  const uint prev1 = idx1 > 0 ? data_[idx1 - 1] : 0;
  const uint prev2 = idx2 > 0 ? data_[idx2 - 1] : 0;
  const uint next1 = idx1 < data_.size() - 1 ? data_[idx1 + 1] : 0;
  const uint next2 = idx2 < data_.size() - 1 ? data_[idx2 + 1] : 0;
  const uint cur1 = data_[idx1];
  const uint cur2 = data_[idx2];

  total_time_ += instance_->getDistance(prev1, cur2) + instance_->getDistance(cur2, next1);
  total_time_ += instance_->getDistance(prev2, cur1) + instance_->getDistance(cur1, next2);
  total_time_ -= (instance_->getDistance(prev1, cur1) + instance_->getDistance(cur1, next1));
  total_time_ -= (instance_->getDistance(prev2, cur2) + instance_->getDistance(cur2, next2));

  data_[idx1] = cur2;
  data_[idx2] = cur1;
}
bool TspIndividualStructured::testSwapMove(uint idx1, uint idx2) {
  assert(std::abs((int)idx1 - (int)idx2) > 1);
  assert(idx1 < data_.size() && idx2 < data_.size());
  const uint prev1 = idx1 > 0 ? data_[idx1 - 1] : 0;
  const uint prev2 = idx2 > 0 ? data_[idx2 - 1] : 0;
  const uint next1 = idx1 < data_.size() - 1 ? data_[idx1 + 1] : 0;
  const uint next2 = idx2 < data_.size() - 1 ? data_[idx2 + 1] : 0;
  const uint cur1 = data_[idx1];
  const uint cur2 = data_[idx2];

  int diff = (int)(instance_->getDistance(prev1, cur2) + instance_->getDistance(cur2, next1));
  diff += (int)(instance_->getDistance(prev2, cur1) + instance_->getDistance(cur1, next2));
  diff -= (int)(instance_->getDistance(prev1, cur1) + instance_->getDistance(cur1, next1));
  diff -= (int)(instance_->getDistance(prev2, cur2) + instance_->getDistance(cur2, next2));

  return diff < 0;
}

void TspIndividualStructured::performRelocateMove(uint idx_from, uint idx_to) {
  assert(idx_from != idx_to);
  const uint prev_from = idx_from > 0 ? data_[idx_from - 1] : 0;
  const uint prev_to = idx_to > 0 ? data_[idx_to - 1] : 0;
  const uint next_from = idx_from < data_.size() - 1 ? data_[idx_from + 1] : 0;
  const uint next_to = idx_to < data_.size() - 1 ? data_[idx_to + 1] : 0;
  const uint cur_to = data_[idx_to];
  const uint cur_from = data_[idx_from];

  if(idx_from < idx_to){
    for(uint i = idx_from; i < idx_to; i++){
      data_[i] = data_[i+1];
    }
    data_[idx_to] = cur_from;
    total_time_ += instance_->getDistance(prev_from, next_from) + instance_->getDistance(cur_from, next_to) + instance_->getDistance(cur_to, cur_from);
    total_time_ -= instance_->getDistance(prev_from, cur_from) + instance_->getDistance(cur_from, next_from) + instance_->getDistance(cur_to, next_to);

  }else if(idx_from > idx_to){
    for(uint i = idx_from; i > idx_to; i--){
      data_[i] = data_[i-1];
    }
    data_[idx_to] = cur_from;
    total_time_ += instance_->getDistance(prev_from, next_from) + instance_->getDistance(cur_from, cur_to) + instance_->getDistance(prev_to, cur_from);
    total_time_ -= instance_->getDistance(prev_from, cur_from) + instance_->getDistance(cur_from, next_from) + instance_->getDistance(prev_to, cur_to);

  }
}

bool TspIndividualStructured::testRelocateMove(uint idx_from, uint idx_to) {
  assert(idx_from != idx_to);
  const uint prev_from = idx_from > 0 ? data_[idx_from - 1] : 0;
  const uint prev_to = idx_to > 0 ? data_[idx_to - 1] : 0;
  const uint next_from = idx_from < data_.size() - 1 ? data_[idx_from + 1] : 0;
  const uint next_to = idx_to < data_.size() - 1 ? data_[idx_to + 1] : 0;
  const uint cur_to = data_[idx_to];
  const uint cur_from = data_[idx_from];


  int diff = 0;
  if(idx_from < idx_to){
    diff += (int)(instance_->getDistance(prev_from, next_from) + instance_->getDistance(cur_from, next_to) + instance_->getDistance(cur_to, cur_from));
    diff -= (int)(instance_->getDistance(prev_from, cur_from) + instance_->getDistance(cur_from, next_from) + instance_->getDistance(cur_to, next_to));
  }else if(idx_from > idx_to){
    diff += (int)(instance_->getDistance(prev_from, next_from) + instance_->getDistance(cur_from, cur_to) + instance_->getDistance(prev_to, cur_from));
    diff -= (int)(instance_->getDistance(prev_from, cur_from) + instance_->getDistance(cur_from, next_from) + instance_->getDistance(prev_to, cur_to));
  }

  return diff < 0;
}

FitnessDiff
TspIndividualStructured::get2optMoveCost(const TspIndividualSegment &segment) {
  assert(segment.start_idx < segment.end_idx);
  assert(segment.start_idx < data_.size());
  assert(segment.end_idx < data_.size());

  const uint high = segment.end_idx;
  const uint low = segment.start_idx;
  const uint prev_node = low > 0 ? data_[low - 1] : 0;
  const uint next_node = high < data_.size() - 1 ? data_[high + 1] : 0;
  const uint prev_distance = instance_->getDistance(prev_node, data_[low]) + instance_->getDistance(data_[high], next_node);
  const uint new_distance = instance_->getDistance(prev_node, data_[high]) + instance_->getDistance(data_[low], next_node);
  return {.fitness = (int)new_distance - (int)prev_distance, .constraints = 0, .vehicles = 0};

}

FitnessDiff TspIndividualStructured::getSwapMoveCost(uint idx1, uint idx2) {
  assert(idx1 < data_.size() && idx2 < data_.size());
  const uint prev1 = idx1 > 0 ? data_[idx1 - 1] : 0;
  const uint prev2 = idx2 > 0 ? data_[idx2 - 1] : 0;
  const uint next1 = idx1 < data_.size() - 1 ? data_[idx1 + 1] : 0;
  const uint next2 = idx2 < data_.size() - 1 ? data_[idx2 + 1] : 0;
  const uint cur1 = data_[idx1];
  const uint cur2 = data_[idx2];

  int diff = (int)(instance_->getDistance(prev1, cur2) + instance_->getDistance(cur2, next1));
  diff += (int)(instance_->getDistance(prev2, cur1) + instance_->getDistance(cur1, next2));
  diff -= (int)(instance_->getDistance(prev1, cur1) + instance_->getDistance(cur1, next1));
  diff -= (int)(instance_->getDistance(prev2, cur2) + instance_->getDistance(cur2, next2));

  return {.fitness = diff, .constraints = 0, .vehicles = 0};
}

FitnessDiff TspIndividualStructured::getRelocateMoveCost(uint idx_from,
                                                         uint idx_to) {
  assert(idx_from != idx_to);
  const uint prev_from = idx_from > 0 ? data_[idx_from - 1] : 0;
  const uint prev_to = idx_to > 0 ? data_[idx_to - 1] : 0;
  const uint next_from = idx_from < data_.size() - 1 ? data_[idx_from + 1] : 0;
  const uint next_to = idx_to < data_.size() - 1 ? data_[idx_to + 1] : 0;
  const uint cur_to = data_[idx_to];
  const uint cur_from = data_[idx_from];


  int diff = 0;
  if(idx_from < idx_to){
    diff += (int)(instance_->getDistance(prev_from, next_from) + instance_->getDistance(cur_from, next_to) + instance_->getDistance(cur_to, cur_from));
    diff -= (int)(instance_->getDistance(prev_from, cur_from) + instance_->getDistance(cur_from, next_from) + instance_->getDistance(cur_to, next_to));
  }else if(idx_from > idx_to){
    diff += (int)(instance_->getDistance(prev_from, next_from) + instance_->getDistance(cur_from, cur_to) + instance_->getDistance(prev_to, cur_from));
    diff -= (int)(instance_->getDistance(prev_from, cur_from) + instance_->getDistance(cur_from, next_from) + instance_->getDistance(prev_to, cur_to));
  }

  return {.fitness = diff, .constraints = 0, .vehicles = 0};
}

FitnessDiff
TspIndividualStructured::getFitnessDiff(const TspIndividualStructured &other) {
  return {.fitness = (int)total_time_ - (int)other.total_time_, .constraints = 0, .vehicles = 0};
}

bool TspIndividualStructured::assertIndividual() {
  if(data_.size() != instance_->getNodesCount() - 1)
    return false;
  std::vector<bool> present(instance_->getNodesCount(), false);
  for(uint i = 0; i < data_.size(); i++){
    if(data_[i] == 0)
      return false;
    if(present[data_[i]])
      return false;
    present[data_[i]] = true;
  }
  return true;
}
