// Copyright 2013-2016 Stanford University
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef STOKE_SRC_COST_LEAKAGE_H
#define STOKE_SRC_COST_LEAKAGE_H

#include <unordered_map>
#include <tuple>
#include <vector>

#include "src/cost/cost_function.h"
#include "src/sandbox/state_callback.h"
#include "src/ext/x64asm/include/x64asm.h"

namespace stoke {

class LeakageCost : public CostFunction {

public:
  LeakageCost() {
    set_run_test_sandbox(true);
  }

  virtual bool need_test_sandbox() {
    return true;
  }

  virtual CostFunction& setup_test_sandbox(Sandbox* sb) {
    test_sandbox_ = sb;
    // Insert the leakage tracking callback
    sb->insert_before(leakage_callback_wrapper, this);
    return *this;
  }

  result_type operator()(const Cfg& cfg, Cost max = max_cost);

  /** Static callback function compatible with StateCallback */
  static void leakage_callback_wrapper(const StateCallbackData& data, void* arg);

private:
  /** Instance method to handle leakage tracking */
  void leakage_callback(const StateCallbackData& data);

  /** Check if any leakage has been detected */
  bool has_leaked() const;

  /** Leakage monitoring map: line number -> (operand0_mask, operand1_mask, operand2_mask) */
  std::unordered_map<int, std::tuple<int, int, int>> leakage_monitor;
};

} // namespace stoke

#endif
