#pragma once
#include "replacement.h"

class TruncationReplacement : public Replacement{

public:
  std::shared_ptr<Population> replacementFunction(const std::shared_ptr<Population> &old_pop, const std::shared_ptr<Population> &new_pop, uint final_size) override;
};
