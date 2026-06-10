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

#include "src/cost/leakage.h"
#include "src/ext/x64asm/include/x64asm.h"
#include <algorithm>

using namespace std;
using namespace x64asm;

namespace stoke {

LeakageCost::result_type LeakageCost::operator()(const Cfg& cfg, Cost max) {
  // Clear previous leakage data
  // Determine cost based on whether leakage was detected
  Cost cost = evaluate_current_leakage();
  num_callbacks = 0;
  leakage_monitor.clear();
  return result_type(true, cost);
}

Cost LeakageCost::evaluate_current_leakage() const {
  switch (reduction_) {
  case LeakageReduction::BINARY:
    return has_leaked() ? 1 : 0;
  case LeakageReduction::N_INSTRUCTIONS:
    return num_leaky_instructions();
  case LeakageReduction::N_EQUIVALENCE_CLASSES:
    return sum_equivalence_classes();
  case LeakageReduction::N_VALUE_RANGES:
    return sum_value_ranges();
  default:
    assert(false);
    return 0;
  }
}

void LeakageCost::leakage_callback_wrapper(const StateCallbackData& data, void* arg) {
  // Cast the arg back to LeakageCost instance
  LeakageCost* cost_func = static_cast<LeakageCost*>(arg);
  cost_func->leakage_callback(data);
}

void LeakageCost::leakage_callback(const StateCallbackData& data) {
  // Track instruction execution for leakage analysis
  const auto& instruction = data.code[data.line];
  auto opcode = instruction.get_opcode();

  // Terminate early if no leakage range data for this opcode
  if (leakage_ranges.find(opcode) == leakage_ranges.end()) {
    return;
  }
  // cout << endl << "Calling back on " << opcode << endl;
  num_callbacks++;

  // Extract mapped operand values from callback state
  std::unordered_map<OperandID, uint64_t> operand_values;
  for (size_t i = 0; i < instruction.arity(); ++i) {
    const auto& op = instruction.get_operand<x64asm::Operand>(i);
    OperandID id = get_operand_id(instruction, i, false);

    // Not handled: memory operands, non-general-purpose registers, flags as implicit inputs
    if (id == OperandID::NotSupported) {
      continue;
    }

    uint64_t value = 0;
    if (op.is_gp_register()) {
      auto& reg = reinterpret_cast<const x64asm::R&>(op);
      switch (x64asm::bit_width_of_type(instruction.type(i))) {
        case 8:
          value = data.state.gp[reg].get_fixed_byte(0);
          break;
        case 16:
          value = data.state.gp[reg].get_fixed_word(0);
          break;
        case 32:
          value = data.state.gp[reg].get_fixed_double(0);
          break;
        case 64:
          value = data.state.gp[reg].get_fixed_quad(0);
          break;
        default:
          assert(false);
      }
    } else if (op.is_immediate()) {
      value = reinterpret_cast<const x64asm::Imm&>(op);
    }
    operand_values[id] = value;
    // cout << "Operand at " << i << ": " << instruction.type(i) << " -> " << id << ", " << value << endl;
  }

  // Record equivalence class and corresponding value ranges
  std::pair<int,uint32_t> eq_class_and_mask = get_equivalence_class_and_partition_mask(operand_values, opcode);
  const int eq_class = eq_class_and_mask.first;
  if (eq_class >= 0) {
    // Initialize leakage_monitor entry if it doesn't exist
    if (leakage_monitor.find(data.line) == leakage_monitor.end() ||
        leakage_monitor[data.line].find(eq_class) == leakage_monitor[data.line].end()) {
      leakage_monitor[data.line][eq_class] = 0;
    }
    leakage_monitor[data.line][eq_class] |= eq_class_and_mask.second;
  }
}

bool LeakageCost::has_leaked() const {
  return num_leaky_instructions() > 0;
}

int LeakageCost::num_leaky_instructions() const {
  int count = 0;
  for (const auto& kv : leakage_monitor) {
    // cout << "Line " << kv.first << " has size " << kv.second.size() << endl;
    if (kv.second.size() > 1) {
      ++count;
    }
  }
  // cout << "Num leaky instructions: " << count << endl << endl;
  return count;
}

int LeakageCost::sum_equivalence_classes() const {
  int sum = 0;
  for (const auto& kv : leakage_monitor) {
    if (kv.second.size() > 1) {
      sum += kv.second.size() - 1;
    }
  }
  return sum;
}

int LeakageCost::sum_value_ranges() const {
  int sum = 0;
  for (const auto& kv : leakage_monitor) {
    if (kv.second.size() > 1) {
      for (const auto& ec : kv.second) {
        sum += popct(ec.second);
      }
    }
  }
  return sum;
}

std::pair<int,uint32_t> LeakageCost::get_equivalence_class_and_partition_mask(const std::unordered_map<OperandID, uint64_t>& values, x64asm::Opcode& opcode) {
  auto it = leakage_ranges.find(opcode);
  if (it == leakage_ranges.end()) {
    return std::make_pair(-1, 0); // No leakage information for this opcode
  }

  const std::vector<EquivalenceClass>& eq_classes = it->second;
  int eq_class = -1;
  uint32_t value_ranges = 100;

  // Iterate over equivalence classes until we find one that matches
  for (size_t i = 0; eq_class < 0 && i < eq_classes.size(); ++i) {
    uint32_t mask = get_partition_index_mask(eq_classes[i], values);
    // cout << "Mask for eq_class " << i << ": " << mask << endl;
    if (mask) {
      eq_class = i;
      value_ranges = mask;
    }
  }

  return std::make_pair(eq_class, value_ranges);
}

uint32_t LeakageCost::get_partition_index_mask(const EquivalenceClass& eq_class, const std::unordered_map<OperandID, uint64_t>& values) {
  uint32_t mask = 0;
  for (size_t i = 0; i < eq_class.partitions.size(); ++i) {
    const auto& partition_map = eq_class.partitions[i];

    // Be conservative: only disqualify a given set of operand values if at least one explicitly falls outside the partition,
    // not if it's ambiguous (i.e., due to missing or unspecified operand values)
    bool all_ops_in_partition = true;
    for (const auto& opv : values) {
      auto it = partition_map.find(opv.first);
      if (it != partition_map.end() && !it->second.contains(opv.second)) {
        // cout << "Op " << opv.first << ", " << opv.second << " not in partition" << endl;
        all_ops_in_partition = false;
        break;
      }
    }

    // Report ALL partitions this set of operand values falls into
    if (all_ops_in_partition) {
      mask |= (1 << i);
    }
  }

  return mask;
}

uint32_t LeakageCost::popct(uint32_t val) const {
  uint32_t popct = 0;
  while (val > 0) {
    ++popct;
    val = val >> 1;
  }
  return popct;
}

} // namespace stoke
