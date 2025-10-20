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
  Cost cost = has_leaked() ? 1 : 0;
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
    leakage_monitor[data.line] = std::make_tuple(0, 0, 0);
  }

  // Check if the instruction is a subq
  if (opcode == SUB_R64_R64 || opcode == SUB_R64_IMM32 || 
      opcode == SUB_R64_M64 || opcode == SUB_M64_IMM32 ||
      opcode == SUB_M64_R64) {
      // Update leakage patterns for each operand
      auto& entry = leakage_monitor[data.line];
      for (size_t i = 0; i < std::min(operand_values.size(), size_t(3)); ++i) {
        int mask = (operand_values[i] == 0) ? 1 : 2;
        switch (i) {
          case 0: std::get<0>(entry) |= mask; break;
          case 1: std::get<1>(entry) |= mask; break;
          case 2: std::get<2>(entry) |= mask; break;
        }
      }
  }
}

bool LeakageCost::has_leaked() const {
  auto is_power_of_2_or_zero = [](int value) {
    return value == 0 || (value > 0 && (value & (value - 1)) == 0);
  };
  
  for (const auto& entry : leakage_monitor) {
    const auto& tuple = entry.second;
    if (!is_power_of_2_or_zero(std::get<0>(tuple)) ||
        !is_power_of_2_or_zero(std::get<1>(tuple)) ||
        !is_power_of_2_or_zero(std::get<2>(tuple))) {
      return true;
    }
  }
  return false;
}

} // namespace stoke
