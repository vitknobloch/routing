#include "VRP-TW/vrptw_mutation_random.h"
#include <cassert>

#define CHANCE_2OPT 0.33
#define CHANCE_EXCHANGE 0.33
#define CHANCE_SHIFT 0.35

VrptwMutationRandom::VrptwMutationRandom(
    const std::shared_ptr<RoutingInstance> &instance) {
  instance_ = instance;
  gen_ = std::mt19937(rand_());
  mutation_rate_ = 1.0;
}

bool VrptwMutationRandom::isInPlace() { return true;}

bool VrptwMutationRandom::mutate(
    const std::shared_ptr<Individual> &individual) {
  const std::shared_ptr<VrptwIndividual> &individual_ = std::static_pointer_cast<VrptwIndividual>(individual);
  bool isEvaluated = true;
  std::uniform_real_distribution dist(0.0, 1.0);
  double random_choice = dist(gen_);
  if(individual_->getTotalConstraintViolation() > 0){
    // Infeasible individual
    assert(false); // not implemented function for infeasible solutions
  }
  else{
    // Feasible individual
    if(random_choice < CHANCE_2OPT)
      isEvaluated = mutate2optInsideRoute(individual_);
    else if(random_choice < CHANCE_2OPT + CHANCE_SHIFT)
      isEvaluated = mutateExchange(individual_);
    else if(random_choice < CHANCE_2OPT + CHANCE_SHIFT + CHANCE_EXCHANGE){
      isEvaluated = mutateShift(individual_);
    }
  }

  return isEvaluated;
}

double VrptwMutationRandom::getMutationRate() { return mutation_rate_;}

void VrptwMutationRandom::setMutationRate(double mutation_rate) {
  mutation_rate_ = mutation_rate;
}

bool VrptwMutationRandom::mutate2optInsideRoute(
    const std::shared_ptr<VrptwIndividual> &individual) {
  //select a route to rearrange
  std::uniform_int_distribution<uint> dist_route_select(0, individual->data().size() - 1);
  uint route_start = dist_route_select(gen_);
  auto &data = individual->data();
  while(!isDepot(data[route_start])){
    route_start = (route_start + 1) % data.size();
  }

  // Find route length and end
  uint route_end = (route_start + 1) % data.size();
  while(!isDepot(data[route_end])){
    route_end = (route_end + 1) % data.size();
  }
  uint route_customer_count = (data.size() + route_end - route_start - 1) % data.size();
  if(route_customer_count == 0)
    return true;

  // Select a segment to reverse
  std::uniform_int_distribution<uint> dist_segment_start(1, route_customer_count);
  uint segment_start = (route_start + dist_segment_start(gen_)) % data.size();
  uint rest_length = (data.size() + route_end - segment_start - 1) % data.size();
  if(rest_length < 2)
    return true;
  std::uniform_int_distribution<uint> dist_segment_length(2, rest_length);
  uint segment_length = dist_segment_length(gen_);
  uint segment_end = (segment_start + segment_length - 1) % data.size();

  // Find time to the segment start
  int i = (route_start + 1) % data.size();
  const auto &nodes = instance_->getNodes();
  uint prev_node = 0;
  uint time = 0;
  while(i != segment_start){
    uint cur_node = data[i];
    time += instance_->getDistance(prev_node, cur_node);
    time = std::max(time, (uint)nodes[cur_node].ready_time);
    time += nodes[cur_node].service_time;

    i = (i + 1) % data.size();
    prev_node = cur_node;
  }

  // establish last node before segment and first after selected route segment
  uint pre_segment_node = prev_node;
  uint post_segment_node = data[(segment_end + 1) % data.size()];
  if(post_segment_node >= instance_->getNodesCount())
    post_segment_node = 0;

  // Find if new order violates due-dates and its time to complete
  uint time_new = time;
  i = segment_end;
  bool in_segment = true;
  while(in_segment){
    if(i == segment_start)
      in_segment = false;
    uint cur_node = data[i];
    time_new += instance_->getDistance(prev_node, cur_node);
    time_new = std::max(time_new, (uint)nodes[cur_node].ready_time);
    if(time_new > nodes[cur_node].due_date){
      return true;
    }
    time_new += nodes[cur_node].service_time;
    i = (i - 1 + (int)data.size()) % (int)data.size();
    prev_node = cur_node;
  }
  time_new += instance_->getDistance(prev_node, post_segment_node);

  // Find original time to complete
  uint time_old = time;
  prev_node = pre_segment_node;
  i = segment_start;
  in_segment = true;
  while(in_segment){
    if(i == segment_end)
      in_segment = false;
    uint cur_node = data[i];
    time_old += instance_->getDistance(prev_node, cur_node);
    time_old = std::max(time_old, (uint)nodes[cur_node].ready_time);
    time_old += nodes[cur_node].service_time;
    i = (i + 1) % (int)data.size();
    prev_node = cur_node;
  }
  time_old += instance_->getDistance(prev_node, post_segment_node);

  if(time_new > time_old)
    return true; // not an improving move

  // reverse segment
  for(int j = 0; j < segment_length / 2; j++){
    const uint &left_idx = (segment_start + j) % data.size();
    const uint &right_idx = (data.size() + segment_end - j) % data.size();
    std::swap(data[left_idx], data[right_idx]);
  }

  individual->resetEvaluated();
  individual->evaluate();
  assert(individual->getTotalConstraintViolation() == 0);
  return true;
}

inline bool VrptwMutationRandom::isCustomer(const uint &node) { return node > 0 && node < (uint)instance_->getNodesCount(); }
inline bool VrptwMutationRandom::isDepot(const uint &node) { return !isCustomer(node); }
