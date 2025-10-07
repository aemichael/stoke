// Copyright 2013-2016 Stanford University
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/cfg/cfg.h"
#include "src/cfg/paths.h"
#include "src/symstate/memory/trivial.h"
#include "src/validator/leakage.h"
#include "src/validator/handler.h"
#include "src/validator/invariants/conjunction.h"
#include "src/validator/invariants/memory_equality.h"
#include "src/validator/invariants/no_signals.h"
#include "src/validator/invariants/state_equality.h"
#include "src/validator/invariants/true.h"

#define LEAKAGE_DEBUG(X) { }

#define MAX(X,Y) ( (X) > (Y) ? (X) : (Y) )
#define MIN(X,Y) ( (X) < (Y) ? (X) : (Y) )

using namespace cpputil;
using namespace std;
using namespace stoke;
using namespace x64asm;


bool LeakageValidator::verify_pair(const Cfg& target, const Cfg& rewrite, const CfgPath& P, const CfgPath& Q) {
  StateEqualityInvariant assume_state(target.def_ins());
  StateEqualityInvariant prove_state(target.live_outs());
  NoSignalsInvariant no_sig;

  MemoryEqualityInvariant memory_equal;

  ConjunctionInvariant assume;
  assume.add_invariant(&assume_state);
  assume.add_invariant(&memory_equal);
  assume.add_invariant(&no_sig);

  ConjunctionInvariant prove;
  prove.add_invariant(&prove_state);
  prove.add_invariant(&memory_equal);

  //LEAKAGE_DEBUG(cout << "[lv] heap/stack out: " << heap_out_ << " " << stack_out_ << endl;)
  bool equiv;
  if (heap_out_ || stack_out_) {
    equiv = check(target, rewrite, target.get_entry(), rewrite.get_entry(), P, Q, assume, prove);
  } else {
    equiv = check(target, rewrite, target.get_entry(), rewrite.get_entry(), P, Q, assume, prove_state);
  }

  if (checker_has_ceg()) {
    assert(!equiv);
    counterexamples_.push_back(checker_get_target_ceg());
    target_final_state_ = checker_get_target_ceg_end();
    rewrite_final_state_ = checker_get_rewrite_ceg_end();
  }

  return equiv;
}


// Check for leakage at the current instruction with the given starting state
bool LeakageValidator::check_leakage(const Cfg& cfg, SymState& state) {
  auto bb = cfg.get_entry();
  if (cfg.num_instrs(bb) != 1) {
    throw VALIDATOR_ERROR("Multiple instructions in leakage query" << endl);
  }

  auto instr = cfg.get_code()[i];
  cout << "INST: " << P[i] << endl << "\t[" << instr << "]" << endl;

  // Collect list of constraint sets, with each top-level element corresponding
  // to the constraints for one equivalence class.
  vector<vector<SymBool>> constraints;
  
  // Build leakage constraints for the current instruction
  string opcode = Handler::get_opcode(instr);

  if (opcode == "subl") {
    Operand src = instr.get_operand<Operand>(1);
    SymBitVector src_bv = state[src];
    uint16_t width = src.size();
    
    cout << "SUBL SRC: " << src << " = " << src_bv << endl;

    // TODO: pull these from generated map
    vector<SymBool> constraints_1;
    constraints_1.push_back(src_bv == SymBitVector::constant(width, 0));
    constraints.push_back(constraints_1);

    vector<SymBool> constraints_2;
    constraints_2.push_back(src_bv != SymBitVector::constant(width, 0));
    constraints.push_back(constraints_2);
  }
  
  // Check all sets of leakage constraints
  for (size_t i = 0; i < constraints.size(); ++i) {
    cout << "Checking leakage for constraint set " << i << endl;

    auto ecs = constraints[i];
    bool has_sat = false;
    bool is_leaky = false;

    for (size_t j = 0; j < ecs.size(); ++j) {
      bool is_sat = solver_.is_sat(ecs[j]);
      if (solver_.has_error()) {
        throw VALIDATOR_ERROR("solver: " + solver_.get_error());
      }

      // "Leaky" means there are possible paths through this instruction that
      // fall into more than one distinguishable equivalence class
      if (is_sat && has_sat) {
        // TODO record counterexamples
        cout << "Found leakage (" << j << ")" << endl;
        is_leaky = true;

      } else if (is_sat) {
        cout << "Found SAT (" << j << ")" << endl;
        has_sat = true;
      }

      // We can finish early once we've determined the instruction is leaky
      if (is_leaky && bailout_)
        break;
    }
    no_lkg &= !is_leaky;
  }

  return no_lkg;
}


bool LeakageValidator::verify_no_leakage(const Cfg& cfg, const CfgPath& P) {
  bool no_lkg = true;

  SymState state("INIT");
  
  // We don't consider memory instructions for leakage, but we do have to model it
  // for accurate data flow
  // Using super simple flat memory model for now
  state.memory = new FlatMemory();

  // Unroll CFG with line numbers for circuit-building
  LineMap line_map;
  rewrite_cfg_with_path(cfg, P, line_map);

  // Step through the path instruction-by-instruction to check for leakage
  size_t line_no = 0;
  for (size_t i = 0; i < P.size(); ++i) {
    auto bb = P[i];
    cout << "Examining BB: " << bb << endl;
    
    // Check input leakage first, then step the state forward
    // Will need changes if we want to check leakage on output too
    no_lkg &= check_leakage(bb, state);
    build_circuit(cfg, bb, is_jump(bb,bb.get_entry(),P,i), state, line_no, line_map);
  }

  return no_lkg;
}



bool LeakageValidator::verify(const Cfg& init_target, const Cfg& init_rewrite) {


#ifdef DEBUG_VALIDATOR
  std::cout << "Enter the dragon!" << std::endl;
#endif
  // State
  counterexamples_.clear();
  leakage_counterexamples_.clear();

  vector<CfgPath> target_paths;
  vector<CfgPath> rewrite_paths;

  has_error_ = false;
  init_mm();

  auto target = inline_functions(init_target);
  auto rewrite = inline_functions(init_rewrite);

  try {

    // Step 0: Background checks
    sanity_checks(target, rewrite);

    // Step 1: get all the paths from the enumerator
    for (auto path : CfgPaths::enumerate_paths(target, bound_)) {
      //cout << "adding TP: " << path << endl;
      target_paths.push_back(path);
    }
    //cout << "REWRITE: " << endl << rewrite.get_code() << endl;
    for (auto path : CfgPaths::enumerate_paths(rewrite, bound_)) {
      //cout << "adding RP: " << path << endl;
      rewrite_paths.push_back(path);
    }

    // Handle the shorter paths first, please
    // [helps find counterexamples sooner]
    auto by_length = [](const CfgPath& lhs, const CfgPath& rhs) {
      return lhs.size() < rhs.size();
    };
    sort(target_paths.begin(), target_paths.end(), by_length);
    sort(rewrite_paths.begin(), rewrite_paths.end(), by_length);

    // Step 2: check each pair of paths
    bool ok = true;
    size_t total = target_paths.size() * rewrite_paths.size();
    size_t count = 0;
    for (auto target_path : target_paths) {
      for (auto rewrite_path : rewrite_paths) {

        LEAKAGE_DEBUG(cout << "[lv] Checking pair: " << target_path << "; " << rewrite_path << endl;)

        count++;
        ok &= verify_pair(target, rewrite, target_path, rewrite_path);

        // Case 1: verify failed and we have ceg; return false
        // Case 2: verify failed and no counterexampe: keep going
        // Case 3: verify worked: keep going

        if (bailout_ && !ok && counterexamples_.size() > 0)
          break;
      }
      if (bailout_ && !ok && counterexamples_.size() > 0)
        break;
    }

    reset_mm();

    // Step 3: if equality check returned ok, we now check leakage for EACH
    // rewrite path (leakage check is independent from target)
    if (ok) {
      bool no_lkg = true;
      for (auto rewrite_path : rewrite_paths) {
        LEAKAGE_DEBUG(cout << "[lv] Checking leakage: " << rewrite_path << endl;)
        
        no_lkg &= verify_no_leakage(rewrite, rewrite_path);

        if (bailout_ && !no_lkg && leakage_counterexamples_.size() > 0)
          break;
      }
    }

    return ok;

  } catch (validator_error e) {
    has_error_ = true;
    error_ = e.get_message();
    error_file_ = e.get_file();
    error_line_ = e.get_line();

    reset_mm();
    return false;
  }

  reset_mm();

  has_error_ = true;
  error_ = "Internal error!  Unexpected control flow.";
  return false;

}


