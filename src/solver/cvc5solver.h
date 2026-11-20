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


#ifndef _STOKE_SRC_SOLVER_CVC5SOLVER_H
#define _STOKE_SRC_SOLVER_CVC5SOLVER_H

#include <map>
#include <string>

#include <cvc5/cvc5.h>

#include "src/solver/smtsolver.h"
#include "src/symstate/bitvector.h"
#include "src/symstate/memo_visitor.h"

namespace stoke {

/** SMT backend using the cvc5 C++ API (replaces the old CVC4 backend). */
class Cvc5Solver : public SMTSolver {

public:
  Cvc5Solver() : SMTSolver(), solver_(tm_), uninterpreted_(false) {
    solver_.setOption("incremental", "true");
    solver_.setOption("produce-models", "true");
    solver_.setLogic("QF_AUFBV");
    apply_timeout();
  }

  SMTSolver& set_timeout(uint64_t ms) {
    timeout_ = ms;
    apply_timeout();
    return *this;
  }

  bool is_sat(const std::vector<SymBool>& constraints);

  bool has_model() const {
    return !uninterpreted_;
  }
  cpputil::BitVector get_model_bv(const std::string& var, uint16_t bits);
  bool get_model_bool(const std::string& var);

  std::pair<std::map<uint64_t, cpputil::BitVector>, uint8_t> get_model_array(const std::string& var, uint16_t key_bits, uint16_t value_bits);

private:

  /** Forget everything from the previous query. */
  void reset() {
    solver_.resetAssertions();
    variables_.clear();
    uninterpreted_ = false;
  }

  /** Push the timeout (ms, 0 = none) into cvc5's per-query time limit. */
  void apply_timeout() {
    solver_.setOption("tlimit-per", std::to_string(timeout_));
  }

  /** Owns all sorts/terms; must outlive (be declared before) the solver. */
  cvc5::TermManager tm_;
  cvc5::Solver solver_;
  bool uninterpreted_;
  std::map<std::string, cvc5::Term> variables_;


  /** This class converts symbolic bit-vectors into cvc5 terms. */
  class ExprConverter : public SymMemoVisitor<cvc5::Term, cvc5::Term, cvc5::Term> {

  public:
    ExprConverter(Cvc5Solver* parent) : tm_(parent->tm_), variables_(parent->variables_),
      uninterpreted_(&(parent->uninterpreted_)) {}

    cvc5::Term operator()(const SymBool& b) {
      return SymMemoVisitor<cvc5::Term, cvc5::Term, cvc5::Term>::operator()(b.ptr);
    }
    cvc5::Term operator()(const SymBitVector& bb) {
      return SymMemoVisitor<cvc5::Term, cvc5::Term, cvc5::Term>::operator()(bb.ptr);
    }
    cvc5::Term operator()(const SymArray& a) {
      return SymMemoVisitor<cvc5::Term, cvc5::Term, cvc5::Term>::operator()(a.ptr);
    }

    /** Visit bit-vector binop */
    cvc5::Term visit_binop(const SymBitVectorBinop * const bv);
    /** Visit bool binop */
    cvc5::Term visit_binop(const SymBoolBinop * const bv);
    /** Visit bit-vector unop */
    cvc5::Term visit_unop(const SymBitVectorUnop * const bv);
    /** Visit bit-vector compare */
    cvc5::Term visit_compare(const SymBoolCompare * const bv);

    /** Visit a bit-vector array lookup */
    cvc5::Term visit(const SymBitVectorArrayLookup * const bv);
    /** Visit a bit-vector constant */
    cvc5::Term visit(const SymBitVectorConstant * const bv);
    /** Visit a bit-vector extract */
    cvc5::Term visit(const SymBitVectorExtract * const bv);
    /** Visit a bit-vector function */
    cvc5::Term visit(const SymBitVectorFunction * const bv);
    /** Visit a bit-vector if-then-else */
    cvc5::Term visit(const SymBitVectorIte * const bv);
    /** Visit a bit-vector sign-extend */
    cvc5::Term visit(const SymBitVectorSignExtend * const bv);
    /** Visit a bit-vector variable */
    cvc5::Term visit(const SymBitVectorVar * const bv);

    /** Visit a boolean array EQ */
    cvc5::Term visit(const SymBoolArrayEq * const b);
    /** Visit a boolean FALSE */
    cvc5::Term visit(const SymBoolFalse * const b);
    /** Visit a boolean NOT */
    cvc5::Term visit(const SymBoolNot * const b);
    /** Visit a boolean TRUE */
    cvc5::Term visit(const SymBoolTrue * const b);
    /** Visit a boolean VAR */
    cvc5::Term visit(const SymBoolVar * const b);

    /** Visit an array STORE */
    cvc5::Term visit(const SymArrayStore * const a);
    /** Visit an array VAR */
    cvc5::Term visit(const SymArrayVar * const a);


    /** See if there's an error */
    bool has_error() {
      return error_.size() > 0;
    }
    /** Get the error */
    std::string error() {
      return error_;
    }

  private:

    /** Reference to parent term manager */
    cvc5::TermManager& tm_;
    /** Reference to variable table */
    std::map<std::string, cvc5::Term>& variables_;
    bool* uninterpreted_;

    /** Keep error messages */
    std::string error_;

    /** Logic to instantiate a new function variable */
    cvc5::Term build_function(const SymBitVectorFunction * const);
    /** Helper function for rotate by a symbolic amount */
    cvc5::Term generic_rot(bool left, const SymBitVectorAbstract * const, cvc5::Term, cvc5::Term);
  };

};

} //namespace stoke

#endif
