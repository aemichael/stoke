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
#include <cassert>
#include <cstdint>
#include <string>

#include "src/cost/cost_function.h"
#include "src/cost/leakage_reduction.h"
#include "src/sandbox/state_callback.h"
#include "src/ext/x64asm/include/x64asm.h"
#include "src/validator/leakage_ranges.h"
#include "src/validator/handler.h"

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
    // Only instrument instructions that have leakage ranges; leakage_callback ignores the rest
    sb->insert_before(leakage_callback_wrapper, this, has_leakage_ranges);
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

  static bool needs_carry(const x64asm::Opcode& opcode) {
    const std::string opc = Handler::get_opcode(opcode);
    return opc.substr(0, 3) == "adc" || opc.substr(0, 3) == "sbb";
  }

private:
  int num_callbacks;

  /** Fixed-capacity map from operand to value; same semantics as unordered_map::operator[] for writes. */
  class OperandValues {
  public:
    typedef std::pair<OperandID, uint64_t> value_type;
    uint64_t& operator[](OperandID id) {
      for (size_t i = 0; i < size_; ++i) {
        if (entries_[i].first == id) {
          return entries_[i].second;
        }
      }
      assert(size_ < kCapacity);
      entries_[size_] = value_type(id, 0);
      return entries_[size_++].second;
    }
    const value_type* begin() const { return entries_; }
    const value_type* end() const { return entries_ + size_; }
  private:
    static const size_t kCapacity = (size_t)OperandID::NotSupported + 1;
    value_type entries_[kCapacity];
    size_t size_ = 0;
  };

  /** True iff leakage_ranges has an entry for this instruction's opcode */
  static bool has_leakage_ranges(const x64asm::Instruction& instr);

  /** needs_carry(), cached per opcode */
  static bool needs_carry_cached(const x64asm::Opcode& opcode);

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
  std::pair<int,uint32_t> get_equivalence_class_and_partition_mask(const OperandValues& values, const std::vector<EquivalenceClass>& eq_classes);

  /**
   * Determine which value range(s) the values fall into within the given equivalence class, represented as a
   * one-hot encoding. Returns 0 if no partition map matches the values.
   */
  uint32_t get_partition_index_mask(const EquivalenceClass& eq_class, const OperandValues& values);

  /** Utility function */
  uint32_t popct(uint32_t val) const;

  /** Method for reducing leakage to a single cost value over multiple test cases */
  LeakageReduction reduction_;

  /** Leakage monitoring map: line number -> (equivalence class index -> value range mask)  */
  std::unordered_map<int, std::unordered_map<int, uint32_t>> leakage_monitor;
};

} // namespace stoke

#endif
