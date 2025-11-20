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
#include "src/validator/leakage_ranges.h"
#include "src/ext/x64asm/include/x64asm.h"
#include <algorithm>

using namespace std;
using namespace x64asm;

namespace stoke {

LeakageCost::result_type LeakageCost::operator()(const Cfg& cfg, Cost max) {
  // Clear previous leakage data
  // Determine cost based on whether leakage was detected
  Cost cost = has_leaked() ? 1 : 0;
  num_callbacks = 0;
  leakage_monitor.clear();
  return result_type(true, cost);
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

  // Extract all operand values
  std::vector<uint64_t> operand_values;
  for (size_t i = 0; i < instruction.arity(); ++i) {
    const auto& op = instruction.get_operand<x64asm::Operand>(i);
    uint64_t value = 0;
    
    if (op.is_typical_memory()) {
      // Memory operand handling not implemented - skip for now
      continue;
    } else if (op.is_gp_register()) {
      auto& reg = reinterpret_cast<const x64asm::R&>(op);
      value = data.state.gp[reg].get_fixed_quad(0);
    } else if (op.is_immediate()) {
      auto& imm = reinterpret_cast<const x64asm::Imm&>(op);
      value = imm;
    }
    
    operand_values.push_back(value);
  }

  // Initialize leakage_monitor entry if it doesn't exist
  if (leakage_monitor.find(data.line) == leakage_monitor.end()) {
    leakage_monitor[data.line] = 0;
  }

  int mask = get_leakage_mask(operand_values, opcode);
  leakage_monitor[data.line] |= mask;
}

bool LeakageCost::has_leaked() const {
  auto is_power_of_2_or_zero = [](int value) {
    return value == 0 || (value > 0 && (value & (value - 1)) == 0);
  };
  
  for (const auto& kv : leakage_monitor) {
    const auto& entry = kv.second;
    if (!is_power_of_2_or_zero(entry)) {
      return true;
    }
  }
  return false;
}

int LeakageCost::get_leakage_mask(std::vector<uint64_t>& values, x64asm::Opcode& opcode) {
  if (values.size() < 1) {
    return 0; // No operand values to check against partition ranges
  }

  // Determine leakage mask based on values and opcode
  auto it = leakage_ranges.find(opcode);
  if (it == leakage_ranges.end()) {
    return 0; // No leakage information for this opcode
  }

  num_callbacks++;

  const std::vector<std::vector<Partition>> partitions = {std::get<0>(it->second),  std::get<1>(it->second)};
  auto partition_size = partitions[0].size();

  for (size_t i = 0; i < partition_size; ++i) {
    // For each partition, check whether ALL operands fall into this partition
    bool all_ops_in_partition = true;

    for (size_t j = 0; j < values.size(); j++) {
      auto value = values[j];
      bool val_in_partition = false;

      for (const auto& range : partitions[j][i].ranges) {
        int low = std::get<0>(range);
        int high = std::get<1>(range);
        if (value < static_cast<uint64_t>(low) || value > static_cast<uint64_t>(high)) {
          val_in_partition = true;
        }
      }

      all_ops_in_partition = all_ops_in_partition && val_in_partition;
    }

    if (all_ops_in_partition)
      return 1 << (i + 1);
  }

  // Value does not fall into any partition
  return 1;
}

} // namespace stoke
