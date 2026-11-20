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


#include "src/solver/cvc5solver.h"

namespace stoke {

TEST(Cvc5SolverTest, TautologyIsSat) {

  auto x = SymBitVector::var(64, "x");
  auto y = SymBitVector::var(64, "y");
  auto z = SymBitVector::var(64, "z");

  auto lhs = (x | y) & z;
  EXPECT_EQ(SymBitVector::AND, lhs.type());

  auto rhs = (x & z) | (y & z);
  EXPECT_EQ(SymBitVector::OR, rhs.type());

  auto eq = lhs == rhs;
  EXPECT_EQ(SymBool::EQ, eq.type());

  auto constraints = std::vector<SymBool>();
  constraints.push_back(eq);

  Cvc5Solver cvc5;
  EXPECT_TRUE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();

}


TEST(Cvc5SolverTest, FalsehoodIsUnSat) {

  auto x = SymBitVector::var(64, "x");
  auto y = SymBitVector::var(64, "y");
  auto z = SymBitVector::var(64, "z");

  auto lhs = (x | y) & z;
  auto rhs = (x & z) | (y & z);
  auto eq = lhs != rhs;

  auto constraints = std::vector<SymBool>();
  constraints.push_back(eq);

  Cvc5Solver cvc5;
  EXPECT_FALSE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();

}

TEST(Cvc5SolverTest, ImpliesWorksUnsat) {

  auto x = SymBool::var("x");
  auto y = SymBool::var("y");

  auto lhs = (!x) | y;
  auto rhs = x.implies(y);

  auto neq = lhs != rhs;

  auto constraints = std::vector<SymBool>();
  constraints.push_back(neq);

  Cvc5Solver cvc5;
  EXPECT_FALSE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();

}

TEST(Cvc5SolverTest, ImpliesWorksSat) {

  auto x = SymBool::var("x");
  auto y = SymBool::var("y");

  auto lhs = (!x) | y;
  auto rhs = x.implies(y);

  auto eq = lhs == rhs;

  auto constraints = std::vector<SymBool>();
  constraints.push_back(eq);

  Cvc5Solver cvc5;
  EXPECT_TRUE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();

}

TEST(Cvc5SolverTest, ConcatTautology) {

  auto x = SymBitVector::var(32, "x");
  auto eq = (x[31][16] || x[15][0]) != x;

  auto constraints = std::vector<SymBool>();
  constraints.push_back(eq);

  Cvc5Solver cvc5;
  EXPECT_FALSE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();
}

TEST(Cvc5SolverTest, ConcatTautology2) {

  auto x = SymBitVector::var(32, "x");
  auto zeros = SymBitVector::constant(16, 0);
  auto eq1 = (zeros || x[15][0]) != x;
  auto eq2 = x[31][16] == zeros;

  auto constraints = std::vector<SymBool>();
  constraints.push_back(eq1);
  constraints.push_back(eq2);

  Cvc5Solver cvc5;
  EXPECT_FALSE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();
}


TEST(Cvc5SolverTest, MulModTest) {

  auto x = SymBitVector::var(8, "x");
  auto y = SymBitVector::var(8, "y");

  auto m = SymBitVector::var(4, "m");
  auto mx = (SymBitVector::constant(4, 0) || m);

  // x*y % m == ((x % m) * (y % m)) % m
  auto lhs = (x * y) % mx;
  auto rhs = ((x % mx) * (y % mx)) % mx;
  auto eq = lhs != rhs;

  // Constrain upper 5 bits of x and y to 0.
  auto zero_x = x[7][3] == SymBitVector::constant(5,0);
  auto zero_y = y[7][3] == SymBitVector::constant(5,0);

  // m may not be zero
  auto m_nonzero = m != SymBitVector::constant(4, 0);

  auto constraints = std::vector<SymBool>();
  constraints.push_back(eq);
  constraints.push_back(m_nonzero);
  constraints.push_back(zero_x);
  constraints.push_back(zero_y);

  Cvc5Solver cvc5;
  EXPECT_FALSE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();

}

TEST(Cvc5SolverTest, UninterpretedFunctionIdentity) {

  auto x = SymBitVector::var(32, "x");
  auto y = SymBitVector::var(32, "y");

  auto f = SymFunction("f", 32, {32, 32});
  auto eq = f(x,y) == f(x,y);

  auto constraints = std::vector<SymBool>();
  constraints.push_back(eq);

  Cvc5Solver cvc5;
  EXPECT_TRUE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();
}

TEST(Cvc5SolverTest, UninterpretedFunctionFail) {

  auto x = SymBitVector::var(32, "x");
  auto y = SymBitVector::var(32, "y");

  auto f = SymFunction("f", 32, {32, 32});
  auto eq = f(x,y) == x + y;

  auto constraints = std::vector<SymBool>();
  constraints.push_back(eq);

  Cvc5Solver cvc5;
  EXPECT_TRUE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();
}

TEST(Cvc5SolverTest, DivWorks) {

  auto x = SymBitVector::var(32, "x");
  auto y = SymBitVector::var(32, "y");
  auto z = SymBitVector::var(32, "z");


  auto constraints = std::vector<SymBool>();
  constraints.push_back(x/y == z);

  Cvc5Solver cvc5;
  EXPECT_TRUE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();
}

TEST(Cvc5SolverTest, RolWorks) {

  auto x = SymBitVector::var(32, "x");
  auto y = SymBitVector::var(32, "y");
  auto z = SymBitVector::var(32, "z");


  auto constraints = std::vector<SymBool>();

  // there's a y so that rotating to the left/right by y gives you back x (e.g. 32)
  constraints.push_back(x.ror(y) == x);
  constraints.push_back(x.rol(y) == x);

  // there's a z so that rotating left/right gives different values)
  auto z_l = x.rol(z);
  auto z_r = x.ror(z);
  constraints.push_back(z_l != x);
  constraints.push_back(z_r != x);
  constraints.push_back(z_l != z_r);

  // x is not 0 or -1; y is not 0.
  constraints.push_back(x != SymBitVector::constant(32, 0));
  constraints.push_back(x != SymBitVector::constant(32, -1));
  constraints.push_back(y != SymBitVector::constant(32, 0));

  Cvc5Solver cvc5;
  EXPECT_TRUE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();
}

TEST(Cvc5SolverTest, RolWorks2) {

  auto x = SymBitVector::var(32, "x");
  auto one = SymBitVector::constant(32, 1);

  auto constraints = std::vector<SymBool>();

  // rotating x by 1 = x implies x = 0 or x = -1.
  constraints.push_back(x.ror(one) == x);

  // x is not 0 or -1
  constraints.push_back(x != SymBitVector::constant(32, 0));
  constraints.push_back(x != SymBitVector::constant(32, -1));

  Cvc5Solver cvc5;
  EXPECT_FALSE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered" << cvc5.get_error();
}

TEST(Cvc5SolverTest, StoreLoadEqual) {

  auto constraints = std::vector<SymBool>();

  auto heap = SymArray::tmp_var(64, 8);
  auto address = SymBitVector::tmp_var(64);
  auto value = SymBitVector::tmp_var(8);

  auto updated_heap = heap.update(address, value);
  auto new_value = updated_heap[address];

  constraints.push_back(value != new_value);

  Cvc5Solver cvc5;
  EXPECT_FALSE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered: " << cvc5.get_error();
}

TEST(Cvc5SolverTest, StoreStoreLoadUnequal) {

  auto constraints = std::vector<SymBool>();

  auto heap = SymArray::tmp_var(64, 8);
  auto address1 = SymBitVector::tmp_var(64);
  auto address2 = SymBitVector::tmp_var(64);
  auto value1 = SymBitVector::tmp_var(8);
  auto value2 = SymBitVector::tmp_var(8);

  auto heap1 = heap.update(address1, value1);
  auto heap2 = heap1.update(address2, value2);
  auto new_value = heap2[address1];

  constraints.push_back(value1 != new_value);

  Cvc5Solver cvc5;
  EXPECT_TRUE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered: " << cvc5.get_error();
}

TEST(Cvc5SolverTest, DifferentStoreStoreLoadEqual) {

  auto constraints = std::vector<SymBool>();

  auto heap = SymArray::tmp_var(64, 8);
  auto address1 = SymBitVector::tmp_var(64);
  auto address2 = SymBitVector::tmp_var(64);
  auto value1 = SymBitVector::tmp_var(8);
  auto value2 = SymBitVector::tmp_var(8);

  auto heap1 = heap.update(address1, value1);
  auto heap2 = heap1.update(address2, value2);
  auto new_value = heap2[address1];

  constraints.push_back(address1 < address2);
  constraints.push_back(value1 != new_value);

  Cvc5Solver cvc5;
  EXPECT_FALSE(cvc5.is_sat(constraints));
  EXPECT_FALSE(cvc5.has_error()) << "cvc5 encountered: " << cvc5.get_error();
}


} //namespace stoke
