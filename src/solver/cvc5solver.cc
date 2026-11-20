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


#include <iostream>
#include <sstream>

#include "src/solver/cvc5solver.h"
#include "src/symstate/bitvector.h"
#include "src/symstate/typecheck_visitor.h"

using namespace stoke;
using namespace std;
using cvc5::Kind;
using cvc5::Term;

#define DEBUG_CVC5(X) { }

namespace {

/** Interpret a (<= 64-bit) bit-vector value term as an unsigned integer. */
uint64_t bv_value_to_u64(const Term& t) {
  return stoull(t.getBitVectorValue(16), nullptr, 16);
}

} // namespace

bool Cvc5Solver::is_sat(const vector<SymBool>& constraints) {

  error_ = "";

  try {
    reset();

    SymTypecheckVisitor tc;
    ExprConverter ec(this);

    for (auto it : constraints) {

      if (tc(it) != 1) {
        stringstream ss;
        ss << "Typechecking failed for constraint: " << it << endl;
        if (tc.has_error())
          ss << "error: " << tc.error() << endl;
        else
          ss << "(no typechecking error message given)" << endl;
        error_ = ss.str();
        return false;
      }

      auto converted = ec(it);
      if (ec.has_error()) {
        error_ = ec.error();
        return false;
      }

      solver_.assertFormula(converted);
    }

    auto result = solver_.checkSat();

    if (result.isUnknown()) {
      error_ = "cvc5 returned unknown: " + result.toString();
      return false;
    }
    return result.isSat();

  } catch (const cvc5::CVC5ApiException& e) {
    error_ = string("cvc5 encountered error: ") + e.what();
    return false;
  }
}

/** Get the satisfying assignment for a bit-vector from the model. */
cpputil::BitVector Cvc5Solver::get_model_bv(const std::string& var, uint16_t bits) {
  cpputil::BitVector bv(bits);

  if (!variables_.count(var))
    return bv;

  // Binary string, most significant bit first.
  auto value = solver_.getValue(variables_[var]).getBitVectorValue(2);
  assert(value.size() == bits);

  for (size_t i = 0; i < bits; ++i) {
    bv[i] = value[bits - 1 - i] == '1';
  }

  assert(bv.num_bits() == bits);
  return bv;
}

/** Get the satisfying assignment for a bit from the model. */
bool Cvc5Solver::get_model_bool(const std::string& var) {

  if (!variables_.count(var))
    return false;

  return solver_.getValue(variables_[var]).getBooleanValue();
}


std::pair<std::map<uint64_t, cpputil::BitVector>, uint8_t> Cvc5Solver::get_model_array(const std::string& var, uint16_t key_bits, uint16_t value_bits) {

  map<uint64_t, cpputil::BitVector> output_map;
  uint8_t default_value = 0;

  if (variables_.count(var)) {

    auto expr = solver_.getValue(variables_[var]);

    DEBUG_CVC5(cout << "[cvc5][model] Expr: " << expr << endl;)

    // Array model values are a chain of STOREs over a constant array.
    while (expr.getKind() == Kind::STORE) {
      uint64_t index = bv_value_to_u64(expr[1]);
      uint64_t value = bv_value_to_u64(expr[2]);
      expr = expr[0];

      cpputil::BitVector bv(8);
      bv.get_fixed_byte(0) = value & 0xff;
      // Outermost store wins; don't let shadowed inner stores overwrite it.
      output_map.emplace(index, bv);
      DEBUG_CVC5(cout << "[cvc5][model] adding " << index << " -> " << value << endl;)
    }

    if (expr.getKind() == Kind::CONST_ARRAY) {
      default_value = bv_value_to_u64(expr.getConstArrayBase());
      DEBUG_CVC5(cout << "[cvc5][model] default value " << (int)default_value << endl;)
    }
  } else {
    cout << "[cvc5][model] WARNING! BUG! Could not find variable " << var << endl;
  }

  return pair<map<uint64_t, cpputil::BitVector>, uint8_t>(output_map, default_value);
}

///////  The following is for converting bit-vectors.  Very tedious.  //////////////////////////////

Term Cvc5Solver::ExprConverter::generic_rot(bool left,
    const SymBitVectorAbstract * const x, Term a, Term b) {

  // cvc5's rotate operators only take constant amounts, so encode a
  // symbolic rotate as a shift over the operand concatenated with itself.

  // Get the size of the argument
  uint16_t size = x->width_;

  // Number of bits to rotate, simplified
  Term amt = tm_.mkTerm(Kind::BITVECTOR_UREM, {b, tm_.mkBitVector(size, (uint64_t)size)});
  // Number of bits to rotate, extended
  Term amt_ext = tm_.mkTerm(tm_.mkOp(Kind::BITVECTOR_ZERO_EXTEND, {size}), {amt});

  // Repeat the source one time
  Term rept = tm_.mkTerm(Kind::BITVECTOR_CONCAT, {a, a});

  if (left) {
    Term tmp = tm_.mkTerm(Kind::BITVECTOR_SHL, {rept, amt_ext});
    return tm_.mkTerm(tm_.mkOp(Kind::BITVECTOR_EXTRACT, {(uint32_t)(2*size-1), size}), {tmp});
  } else {
    Term tmp = tm_.mkTerm(Kind::BITVECTOR_LSHR, {rept, amt_ext});
    return tm_.mkTerm(tm_.mkOp(Kind::BITVECTOR_EXTRACT, {(uint32_t)(size-1), 0}), {tmp});
  }
}

/** Visit a bit-vector binop */
Term Cvc5Solver::ExprConverter::visit_binop(const SymBitVectorBinop * const binop) {
  auto left = (*this)(binop->a_);
  auto right = (*this)(binop->b_);

  switch (binop->type()) {
  case SymBitVector::AND:
    return tm_.mkTerm(Kind::BITVECTOR_AND, {left, right});
  case SymBitVector::CONCAT:
    return tm_.mkTerm(Kind::BITVECTOR_CONCAT, {left, right});
  case SymBitVector::MINUS:
    return tm_.mkTerm(Kind::BITVECTOR_SUB, {left, right});
  case SymBitVector::DIV:
    return tm_.mkTerm(Kind::BITVECTOR_UDIV, {left, right});
  case SymBitVector::MOD:
    return tm_.mkTerm(Kind::BITVECTOR_UREM, {left, right});
  case SymBitVector::MULT:
    return tm_.mkTerm(Kind::BITVECTOR_MULT, {left, right});
  case SymBitVector::OR:
    return tm_.mkTerm(Kind::BITVECTOR_OR, {left, right});
  case SymBitVector::PLUS:
    return tm_.mkTerm(Kind::BITVECTOR_ADD, {left, right});
  case SymBitVector::ROTATE_LEFT:
    return generic_rot(true, binop->a_, left, right);
  case SymBitVector::ROTATE_RIGHT:
    return generic_rot(false, binop->a_, left, right);
  case SymBitVector::SHIFT_LEFT:
    return tm_.mkTerm(Kind::BITVECTOR_SHL, {left, right});
  case SymBitVector::SHIFT_RIGHT:
    return tm_.mkTerm(Kind::BITVECTOR_LSHR, {left, right});
  case SymBitVector::SIGN_DIV:
    return tm_.mkTerm(Kind::BITVECTOR_SDIV, {left, right});
  case SymBitVector::SIGN_MOD:
    return tm_.mkTerm(Kind::BITVECTOR_SREM, {left, right});
  case SymBitVector::SIGN_SHIFT_RIGHT:
    return tm_.mkTerm(Kind::BITVECTOR_ASHR, {left, right});
  case SymBitVector::XOR:
    return tm_.mkTerm(Kind::BITVECTOR_XOR, {left, right});
  default:
    error_ = "Unexpected BV type: " + to_string(binop->type());
    assert(false);
  }
  assert(false);
  return tm_.mkTerm(Kind::BITVECTOR_AND, {left, right}); // keep the compiler happy
}

/** Visit a bit-vector unop */
Term Cvc5Solver::ExprConverter::visit_unop(const SymBitVectorUnop * const unop) {
  auto left = (*this)(unop->bv_);

  switch (unop->type()) {
  case SymBitVector::U_MINUS:
    return tm_.mkTerm(Kind::BITVECTOR_NEG, {left});
  case SymBitVector::NOT:
    return tm_.mkTerm(Kind::BITVECTOR_NOT, {left});
  default:
    error_ = "Unexpected BV type: " + to_string(unop->type());
    assert(false);
  }
  assert(false);
  return tm_.mkTerm(Kind::BITVECTOR_NEG, {left}); //keep the compiler happy
}

/** Visit a boolean binop */
Term Cvc5Solver::ExprConverter::visit_binop(const SymBoolBinop * const binop) {
  auto left = (*this)(binop->a_);
  auto right = (*this)(binop->b_);

  switch (binop->type()) {
  case SymBool::AND:
    return tm_.mkTerm(Kind::AND, {left, right});
  case SymBool::IFF:
    return tm_.mkTerm(Kind::EQUAL, {left, right});
  case SymBool::IMPLIES:
    return tm_.mkTerm(Kind::IMPLIES, {left, right});
  case SymBool::OR:
    return tm_.mkTerm(Kind::OR, {left, right});
  case SymBool::XOR:
    return tm_.mkTerm(Kind::XOR, {left, right});
  default:
    error_ = "Unexpected bool type: " + to_string(binop->type());
    assert(false);
  }
  assert(false);
  return tm_.mkTerm(Kind::AND, {left, right}); //keep the compiler happy
}

/** Visit a binop compare  */
Term Cvc5Solver::ExprConverter::visit_compare(const SymBoolCompare * const compare) {
  auto left = (*this)(compare->a_);
  auto right = (*this)(compare->b_);

  switch (compare->type()) {
  case SymBool::EQ:
    return tm_.mkTerm(Kind::EQUAL, {left, right});
  case SymBool::GE:
    return tm_.mkTerm(Kind::BITVECTOR_UGE, {left, right});
  case SymBool::GT:
    return tm_.mkTerm(Kind::BITVECTOR_UGT, {left, right});
  case SymBool::LE:
    return tm_.mkTerm(Kind::BITVECTOR_ULE, {left, right});
  case SymBool::LT:
    return tm_.mkTerm(Kind::BITVECTOR_ULT, {left, right});
  case SymBool::SIGN_GE:
    return tm_.mkTerm(Kind::BITVECTOR_SGE, {left, right});
  case SymBool::SIGN_GT:
    return tm_.mkTerm(Kind::BITVECTOR_SGT, {left, right});
  case SymBool::SIGN_LE:
    return tm_.mkTerm(Kind::BITVECTOR_SLE, {left, right});
  case SymBool::SIGN_LT:
    return tm_.mkTerm(Kind::BITVECTOR_SLT, {left, right});
  default:
    error_ = "Unexpected compare type: " + to_string(compare->type());
    assert(false);
  }
  assert(false);
  return tm_.mkTerm(Kind::EQUAL, {left, right});
}

/** Visit a bit-vector array lookup */
Term Cvc5Solver::ExprConverter::visit(const SymBitVectorArrayLookup * const bv) {
  return tm_.mkTerm(Kind::SELECT, {(*this)(bv->a_), (*this)(bv->key_)});
}

/** Visit a bit-vector constant */
Term Cvc5Solver::ExprConverter::visit(const SymBitVectorConstant * const bv) {
  return tm_.mkBitVector(bv->size_, (uint64_t)bv->constant_);
}

/** Visit a bit-vector extract */
Term Cvc5Solver::ExprConverter::visit(const SymBitVectorExtract * const bv) {
  auto op = tm_.mkOp(Kind::BITVECTOR_EXTRACT, {(uint32_t)bv->high_bit_, (uint32_t)bv->low_bit_});
  return tm_.mkTerm(op, {(*this)(bv->bv_)});
}

Term Cvc5Solver::ExprConverter::build_function(const SymBitVectorFunction * const bv) {

  *uninterpreted_ = true;

  auto f = bv->f_;
  auto args = f.args;
  auto ret = f.return_type;

  // get cvc5 representation of the argument/return types
  vector<cvc5::Sort> sorts;
  for (uint16_t it : args) {
    sorts.push_back(tm_.mkBitVectorSort(it));
  }
  auto ret_sort = tm_.mkBitVectorSort(ret);

  if (sorts.size() == 0) {
    error_ = "Function " + f.name + " has no arguments: " + to_string(sorts.size());
    assert(false);
  }

  if (sorts.size() > 3) {
    error_ = "Function " + f.name + " has too many arguments: " + to_string(sorts.size());
    assert(false);
  }

  auto type = tm_.mkFunctionSort(sorts, ret_sort);
  return tm_.mkConst(type, f.name);
}

/** Visit an uninterpreted function */
Term Cvc5Solver::ExprConverter::visit(const SymBitVectorFunction * const bv) {

  if (!variables_.count(bv->f_.name)) {
    variables_[bv->f_.name] = build_function(bv);
  }

  std::vector<Term> args;
  args.push_back(variables_[bv->f_.name]);
  for (auto it : bv->args_) {
    args.push_back((*this)(it));
  }

  return tm_.mkTerm(Kind::APPLY_UF, args);
}

/** Visit a bit-vector if-then-else */
Term Cvc5Solver::ExprConverter::visit(const SymBitVectorIte * const bv) {
  return tm_.mkTerm(Kind::ITE, {(*this)(bv->cond_), (*this)(bv->a_), (*this)(bv->b_)});
}

/** Visit a bit-vector sign extension */
Term Cvc5Solver::ExprConverter::visit(const SymBitVectorSignExtend * const bv) {
  uint16_t size = bv->bv_->width_;
  uint32_t amt = bv->size_ - size;
  return tm_.mkTerm(tm_.mkOp(Kind::BITVECTOR_SIGN_EXTEND, {amt}), {(*this)(bv->bv_)});
}

/** Visit a bit-vector variable */
Term Cvc5Solver::ExprConverter::visit(const SymBitVectorVar * const bv) {
  if (!variables_.count(bv->name_)) {
    variables_[bv->name_] = tm_.mkConst(tm_.mkBitVectorSort(bv->size_), bv->name_);
  }
  return variables_[bv->name_];
}

/** Visit a boolean ARRAY EQ */
Term Cvc5Solver::ExprConverter::visit(const SymBoolArrayEq * const b) {
  return tm_.mkTerm(Kind::EQUAL, {(*this)(b->a_), (*this)(b->b_)});
}

/** Visit a boolean FALSE */
Term Cvc5Solver::ExprConverter::visit(const SymBoolFalse * const b) {
  return tm_.mkFalse();
}

/** Visit a boolean NOT */
Term Cvc5Solver::ExprConverter::visit(const SymBoolNot * const b) {
  return tm_.mkTerm(Kind::NOT, {(*this)(b->b_)});
}

/** Visit a boolean TRUE */
Term Cvc5Solver::ExprConverter::visit(const SymBoolTrue * const b) {
  return tm_.mkTrue();
}

/** Visit a boolean VAR */
Term Cvc5Solver::ExprConverter::visit(const SymBoolVar * const b) {
  if (!variables_.count(b->name_)) {
    variables_[b->name_] = tm_.mkConst(tm_.getBooleanSort(), b->name_);
  }
  return variables_[b->name_];
}

Term Cvc5Solver::ExprConverter::visit(const SymArrayStore * const bv) {
  return tm_.mkTerm(Kind::STORE, {(*this)(bv->a_), (*this)(bv->key_), (*this)(bv->value_)});
}

Term Cvc5Solver::ExprConverter::visit(const SymArrayVar * const bv) {
  if (!variables_.count(bv->name_)) {
    DEBUG_CVC5(cout << "[cvc5] generating array variable " << bv->name_ << endl;)
    auto type = tm_.mkArraySort(tm_.mkBitVectorSort(bv->key_size_), tm_.mkBitVectorSort(bv->value_size_));
    variables_[bv->name_] = tm_.mkConst(type, bv->name_);
  } else {
    DEBUG_CVC5(cout << "[cvc5] using array variable " << bv->name_ << endl;)
  }
  return variables_[bv->name_];
}
