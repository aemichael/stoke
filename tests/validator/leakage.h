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

#include <regex>

#include "src/sandbox/sandbox.h"
#include "src/solver/solver.h"
#include "src/validator/leakage.h"
#include "src/validator/filters/forbidden_dereference.h"
#include "src/validator/invariants/conjunction.h"
#include "src/validator/invariants/equality.h"
#include "src/validator/invariants/no_signals.h"
#include "src/validator/invariants/state_equality.h"
#include "src/validator/invariants/top_zero.h"
#include "src/validator/invariants/true.h"

namespace stoke {

class LeakageValidatorTest : public ::testing::TestWithParam<std::tr1::tuple<ObligationChecker::AliasStrategy, Solver>> {

public:

  LeakageValidatorTest() {
    auto param = ::testing::TestWithParam<std::tr1::tuple<ObligationChecker::AliasStrategy, Solver>>::GetParam();
    auto solver_type = std::tr1::get<1>(GetParam());
    // if (solver_type == Solver::Z3) {
      std::cout << "Using Z3" << std::endl;
      solver = new Z3Solver();
    // }
    // else if (solver_type == Solver::CVC4) {
    //   std::cout << "Using CVC4" << std::endl;
    //   solver = new Cvc4Solver();
    // }

    std::cout << "Alias Strategy " << std::tr1::get<0>(param) << std::endl;

    sandbox = new Sandbox();
    sandbox->set_max_jumps(4096);
    sandbox->set_abi_check(false);
    sg_sandbox = new Sandbox();
    sg_sandbox->set_max_jumps(4096);
    sg_sandbox->set_abi_check(false);

    handler = new ComboHandler();
    vector<uint64_t> low_addrs = {0, (uint64_t)(-0x100)};
    vector<uint64_t> high_addrs = {0x100, (uint64_t)(-1)};
    filter = new ForbiddenDereferenceFilter(*handler, low_addrs, high_addrs);

    validator = new LeakageValidator(*solver);
    validator->set_bound(2);
    validator->set_filter(filter);
    validator->set_sandbox(sandbox);
    validator->set_alias_strategy(std::tr1::get<0>(param));
    validator->set_heap_out(true);
    validator->set_stack_out(true);
  }

  ~LeakageValidatorTest() {
    delete validator;
    delete sandbox;
    delete sg_sandbox;
    delete solver;
    delete handler;
  }

protected:

  static x64asm::RegSet all() {
    auto rs = x64asm::RegSet::all_gps() | x64asm::RegSet::all_ymms();
    rs = rs + x64asm::eflags_cf + x64asm::eflags_zf + x64asm::eflags_pf + x64asm::eflags_of + x64asm::eflags_sf;
    return rs;
  }

  static x64asm::RegSet omit_reserved_and_flags() {
    auto rs = (x64asm::RegSet::all_gps() | x64asm::RegSet::all_ymms());
    rs -= (x64asm::RegSet::empty() + x64asm::Constants::r11());
    return rs;
  }

  static x64asm::RegSet omit_caller_saved() {
    auto rs = (x64asm::RegSet::linux_call_preserved() | x64asm::RegSet::linux_call_return());
    return rs;
  }

  void fail() {
    FAIL();
  }

  void check_ceg(const CpuState& tc, const Cfg& target, const Cfg& rewrite, bool print = false) {
    Sandbox sb;
    sb.set_max_jumps(4096);
    sb.set_abi_check(false);
    sb.insert_input(tc);

    sb.insert_function(target);
    sb.set_entrypoint(target.get_code()[0].get_operand<x64asm::Label>(0));

    sb.run();
    auto target_output = *sb.get_output(0);

    sb.insert_function(rewrite);
    sb.set_entrypoint(rewrite.get_code()[0].get_operand<x64asm::Label>(0));

    sb.run();
    auto rewrite_output = *sb.get_output(0);

    EXPECT_EQ(ErrorCode::NORMAL, target_output.code);
    EXPECT_NE(target_output, rewrite_output);

    if (print) {
      std::cout << "Counterexample:" << std::endl << tc << std::endl;
      std::cout << "Target state:" << std::endl << target_output << std::endl;
      std::cout << "Rewrite state:" << std::endl << rewrite_output << std::endl;
    }
  }

  Cfg make_cfg(std::stringstream& ss, x64asm::RegSet di = all(), x64asm::RegSet lo = all(), uint64_t rip_offset = 0) {
    x64asm::Code c;
    ss >> c;
    if (ss.fail()) {
      std::cerr << "Parsing error!" << std::endl;
      std::cerr << cpputil::fail_msg(ss) << std::endl;
      fail();
    }
    TUnit fxn(c, 0, rip_offset, 0);
    return Cfg(fxn, di, lo);
  }

  CpuState get_state() {
    CpuState cs;
    StateGen sg(sg_sandbox);
    sg.get(cs);
    return cs;
  }

  CpuState get_state(const Cfg& cfg) {
    CpuState cs;
    StateGen sg(sg_sandbox);
    bool b = sg.get(cs, cfg);
    if (!b) {
      std::cerr << "Couldn't generate a state!" << std::endl;
      std::cerr << sg.get_error() << std::endl;
      fail();
    }
    return cs;
  }

  SMTSolver* solver;
  LeakageValidator* validator;
  Sandbox* sandbox;
  Sandbox* sg_sandbox;
  Handler* handler;
  Filter* filter;

};

TEST_P(LeakageValidatorTest, SimpleSublLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "subl %ecx, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "subl %ecx, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, SublConstNotLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "subl $1, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "subl $1, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_TRUE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, SublConstZeroNotLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "subl $0, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "subl $0, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);
  
  EXPECT_TRUE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, SublConstRegNotLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movl $5, %ecx" << std::endl;
  sst << "subl %ecx, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movl $5, %ecx" << std::endl;
  ssr << "subl %ecx, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_TRUE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, SublTransformNotLeaky) {

  auto live_outs = omit_reserved_and_flags();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "subl %ecx, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movq %rcx, %r11" << std::endl;
  ssr << "movl %ecx, %ecx" << std::endl;
  ssr << "subq $0x80000000, %rcx" << std::endl;
  ssr << "subq $0x80000000, %rcx" << std::endl;
  ssr << "subq %rcx, %rax" << std::endl;
  ssr << "movl %eax, %eax" << std::endl;
  ssr << "movq %r11, %rcx" << std::endl;
  ssr << "retq" << std::endl;

  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_TRUE(validator->verify(target, rewrite));
  
  if (validator->counter_examples_available()) {
    for (auto it : validator->get_counter_examples())
      check_ceg(it, target, rewrite, true);
  }

  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, SimpleAddlLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "addl %ecx, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "addl %ecx, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AddlDstOperandLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movl $5, %ecx" << std::endl;
  sst << "addl %ecx, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movl $5, %ecx" << std::endl;
  ssr << "addl %ecx, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AddlSrcOperandLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movl $5, %eax" << std::endl;
  sst << "addl %ecx, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movl $5, %eax" << std::endl;
  ssr << "addl %ecx, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AddlConstOpcodeLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "addl $5, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "addl $5, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AddlConstsNotLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movl $5, %eax" << std::endl;
  sst << "addl $0, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movl $5, %eax" << std::endl;
  ssr << "addl $0, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_TRUE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AddlTransformNotLeaky) {

  auto live_outs = omit_reserved_and_flags();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "addl %ecx, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movq %rcx, %r11" << std::endl;
  ssr << "movl %ecx, %ecx" << std::endl;
  ssr << "subq $0x80000000, %rcx" << std::endl;
  ssr << "subq $0x80000000, %rcx" << std::endl;
  ssr << "movl %eax, %eax" << std::endl;
  ssr << "subq $0x80000000, %rax" << std::endl;
  ssr << "subq $0x80000000, %rax" << std::endl;
  ssr << "addq %rcx, %rax" << std::endl;
  ssr << "movl %eax, %eax" << std::endl;
  ssr << "movq %r11, %rcx" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_TRUE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AddqDstOperandLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movq $5, %rcx" << std::endl;
  sst << "addq %rcx, %rax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movq $5, %rcx" << std::endl;
  ssr << "addq %rcx, %rax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AddqSrcOperandLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movq $5, %rax" << std::endl;
  sst << "addq %rcx, %rax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movq $5, %rax" << std::endl;
  ssr << "addq %rcx, %rax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AddbDstOperandLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movb $5, %cl" << std::endl;
  sst << "addb %cl, %al" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movb $5, %cl" << std::endl;
  ssr << "addb %cl, %al" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AddbSrcOperandLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movb $5, %al" << std::endl;
  sst << "addb %cl, %al" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movb $5, %al" << std::endl;
  ssr << "addb %cl, %al" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AddwDstOperandLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movw $5, %cx" << std::endl;
  sst << "addw %cx, %ax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movw $5, %cx" << std::endl;
  ssr << "addw %cx, %ax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AddwSrcOperandLeaky) {

  auto live_outs = all();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movw $5, %ax" << std::endl;
  sst << "addw %cx, %ax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movw $5, %ax" << std::endl;
  ssr << "addw %cx, %ax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, SimpleNeglLeaky) {

  auto live_outs = omit_caller_saved();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "negl %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "negl %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AndlSrcOperandLeaky) {

  auto live_outs = omit_caller_saved();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movl $0xFF, %eax" << std::endl;
  sst << "andl %ecx, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movl $0xFF, %eax" << std::endl;
  ssr << "andl %ecx, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, AndlDstOperandLeaky) {

  auto live_outs = omit_caller_saved();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movl $0xFF, %ecx" << std::endl;
  sst << "andl %ecx, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movl $0xFF, %ecx" << std::endl;
  ssr << "andl %ecx, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, ShllBy1Leaky) {

  auto live_outs = omit_caller_saved();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "shll $1, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "shll $1, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, ShllByImmLeaky) {

  auto live_outs = omit_caller_saved();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "shll $8, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "shll $8, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

TEST_P(LeakageValidatorTest, ShllByClLeaky) {

  auto live_outs = omit_caller_saved();

  std::stringstream sst;
  sst << ".foo:" << std::endl;
  sst << "movb $8, %cl" << std::endl;
  sst << "shll %cl, %eax" << std::endl;
  sst << "retq" << std::endl;
  auto target = make_cfg(sst, live_outs, live_outs);

  std::stringstream ssr;
  ssr << ".foo:" << std::endl;
  ssr << "movb $8, %cl" << std::endl;
  ssr << "shll $8, %eax" << std::endl;
  ssr << "retq" << std::endl;
  auto rewrite = make_cfg(ssr, live_outs, live_outs);

  EXPECT_FALSE(validator->verify(target, rewrite));
  EXPECT_FALSE(validator->has_error()) << validator->error();
}

INSTANTIATE_TEST_CASE_P(AllSolversAliasing, LeakageValidatorTest,
                        ::testing::Combine(
                          ::testing::Values(ObligationChecker::AliasStrategy::FLAT, ObligationChecker::AliasStrategy::ARM),
                          ::testing::Values(Solver::Z3, Solver::CVC4)
                        )
                       );



} //namespace stoke
