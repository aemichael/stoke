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
#include "src/cost/leakage_reduction.h"
#include "src/sandbox/state_callback.h"
#include "src/ext/x64asm/include/x64asm.h"
#include "src/validator/leakage_ranges.h"

namespace stoke {

class LeakageCost : public CostFunction {

public:
  LeakageCost() {
    set_run_test_sandbox(true);
    num_callbacks = 0;
    set_reduction(LeakageReduction::BINARY);
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

  /** Check if any leakage has been detected */
  bool has_leaked() const;

  /** Static callback function compatible with StateCallback */
  static void leakage_callback_wrapper(const StateCallbackData& data, void* arg);

  /** Set reduction method */
  LeakageCost& set_reduction(LeakageReduction r) {
    reduction_ = r;
    return *this;
  }

private:
  int num_callbacks;

  /** Instance method to handle leakage tracking */
  void leakage_callback(const StateCallbackData& data);

  /** Evaluate leakage based on current leakage monitor state */
  Cost evaluate_current_leakage() const;

  /** Count the number of lines, i.e., instructions, with detected leakage */
  int num_leaky_instructions() const;

  /** Count the number of equivalence classes crossed across all instructions */
  int sum_equivalence_classes() const;

  /** Count the number of value ranges across all equivalence classes and instructions */
  int sum_value_ranges() const;

  /** 
   * Determine equivalence class index (first element) and mask representing the value range(s) within that class. 
   * If there is no value range information for this opcode, returns (-1, 0).
   * If there is value range information but no equivalence class matches, returns (-1, nonzero).
  */
  std::pair<int,uint32_t> get_equivalence_class_and_partition_mask(const std::unordered_map<OperandID, uint64_t>& values, x64asm::Opcode& opcode);

  /** 
   * Determine which value range(s) the values fall into within the given equivalence class, represented as a
   * one-hot encoding. Returns 0 if no partition map matches the values.
   */
  uint32_t get_partition_index_mask(const EquivalenceClass& eq_class, const std::unordered_map<OperandID, uint64_t>& values);

  /** Utility function */
  uint32_t popct(uint32_t val) const;

  /** Method for reducing leakage to a single cost value over multiple test cases */
  LeakageReduction reduction_;

  /** Leakage monitoring map: line number -> (equivalence class index -> value range mask)  */
  std::unordered_map<int, std::unordered_map<int, uint32_t>> leakage_monitor;
};

} // namespace stoke

#endif
