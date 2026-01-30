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

#include "src/transform/opcode_width.h"

using namespace std;
using namespace stoke;
using namespace x64asm;


namespace stoke {

TransformInfo OpcodeWidthTransform::operator()(Cfg& cfg) {

  TransformInfo ti;
  ti.success = false;

  // Grab the index of a random instruction
  ti.undo_index[0] = (gen_() % (cfg.get_code().size() - 1)) + 1;
  Cfg::id_type bb = cfg.get_entry();
  size_t block_idx = 0;
  if (!get_indices(cfg, bb, block_idx, ti.undo_index[0])) {
    return ti;
  }

  ti.undo_instr = cfg.get_code()[ti.undo_index[0]];
  if (is_control_other_than_call(ti.undo_instr.get_opcode()))
    return ti;

  // Try generating a new instruction
  auto instr = ti.undo_instr;
  // cout << "[transform] Applying opcode width transform to instr: " << instr << endl;

  auto opc = instr.get_opcode();
  if (!pools_.get_equivalent_raw_memonic(opc)) {
    return ti;
  }
  instr.set_opcode(opc);
  // if (!instr.is_label_defn()) {
  //   for (size_t j = 0; j < instr.arity(); j++) {
  //     auto op = instr.get_operand<Operand>(j);
  //     cout << " Operand " << j << ": " << op << ":" << op.type() << endl;
  //     if (op.type() != instr.type(j)) {
  //       cout << "    TYPE MISMATCH: Expected " << instr.type(j) << ", got " << op.type() << endl;
  //     }
  //   }
  // }

  const auto& rs = cfg.def_ins({bb, block_idx});
  for (size_t i = 0, ie = instr.arity(); i < ie; ++i) {
    Operand o = instr.get_operand<R64>(i);
    if (instr.maybe_read(i)) {
      if (!pools_.get_read_op(instr.get_opcode(), i, rs, o)) {
        return ti;
      }
    } else {
      if (!pools_.get_write_op(instr.get_opcode(), i, rs, o)) {
        return ti;
      }
    }
    instr.set_operand(i, o);
  }

  // Check that the instruction is valid
  if (!instr.check()) {
    // cout << "Invalid!" << endl << endl;
    return ti;
  }
  // cout << "Valid! " << instr << endl;

  // Success: Any failure beyond here will require undoing the move
  cfg.get_function().replace(ti.undo_index[0], instr, false, true);
  cfg.recompute_defs();
  if (!cfg.check_invariants()) {
    undo(cfg, ti);
    return ti;
  }

  // if (!instr.is_label_defn()) {
  //   for (size_t j = 0; j < instr.arity(); j++) {
  //     auto op = instr.get_operand<Operand>(j);
  //     cout << " Operand " << j << ": " << op << ":" << op.type() << endl;
  //     if (op.type() != instr.type(j)) {
  //       cout << "    TYPE MISMATCH: Expected " << instr.type(j) << ", got " << op.type() << endl;
  //     }
  //   }
  // }
  // cout << endl;

  assert(cfg.invariant_no_undef_reads());
  assert(cfg.get_function().check_invariants());


  ti.success = true;
  return ti;
}

void OpcodeWidthTransform::undo(Cfg& cfg, const TransformInfo& ti) const {
  cfg.get_function().replace(ti.undo_index[0], ti.undo_instr, true);
  cfg.recompute_defs();

  assert(cfg.invariant_no_undef_reads());
  assert(cfg.get_function().check_invariants());


}



} // namespace stoke
