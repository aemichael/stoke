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


#include <sstream>

#include "src/cfg/cfg.h"
#include "src/cost/leakage.h"
#include "src/cost/leakage_reduction.h"
#include "src/ext/x64asm/include/x64asm.h"
#include "src/sandbox/sandbox.h"
#include "src/state/cpu_state.h"
#include "src/stategen/stategen.h"

namespace stoke {

class LeakageCostTest : public ::testing::Test {

public:
  LeakageCostTest() : fxn_() {
    sb_.set_abi_check(false);
    fxn_.setup_test_sandbox(&sb_);
  }

protected:

  void add_testcases_for_zero(int count) {
    for (int i = 0; i < count; ++i) {
      auto state = get_state();
      
      // The 10th testcase will set all registers to zero to trigger leakage
      if (i == 9) {
        for (size_t j = 0; j < state.gp.size(); ++j) {
          state.gp[j].get_fixed_quad(0) = 0;
        }
      }
      
      // std::cout << "Adding testcase " << i << ": " << state << std::endl;

      sb_.insert_input(state);
    }
  }

  CpuState get_state() {
    CpuState cs;
    StateGen sg(&sb_);
    sg.get(cs);
    return cs;
  }

  Cfg make_cfg(x64asm::Code c, x64asm::RegSet rs = x64asm::RegSet::universe()) {
    return Cfg(c, rs, rs);
  }

  Sandbox sb_;
  LeakageCost fxn_;

private:
  void SetUp() {
    // Setup sandbox for leakage cost testing
    sb_.set_max_jumps(1024);
    sb_.set_abi_check(false);
  }

};

TEST_F(LeakageCostTest, SingleSubqLeaks) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "subq %rbx, %rax" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(1ul, result.second);  // Cost should be 1 for leakage
}

TEST_F(LeakageCostTest, SublTransformDoesntLeak) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "movq %rcx, %r11" << std::endl;
  ss << "movl %ecx, %ecx" << std::endl;
  ss << "subq $0x80000000, %rcx" << std::endl;
  ss << "subq $0x80000000, %rcx" << std::endl;
  ss << "subq %rcx, %rax" << std::endl;
  ss << "movl %eax, %eax" << std::endl;
  ss << "movq %r11, %rcx" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  auto result = fxn_(cfg);

  // Expect the cost to be 0 (indicating no leakage detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(0ul, result.second);  // Cost should be 0 for no leakage
}

TEST_F(LeakageCostTest, SingleSubqNoZeroCaseDoesntLeak) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(9);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "subq %rbx, %rax" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  auto result = fxn_(cfg);

  // Expect the cost to be 0 as the last testcase does not exist to trigger leakage.
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(0ul, result.second);  // Cost should be 0 for no leakage
}

TEST_F(LeakageCostTest, AndqZeroSrcLeaks) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "movl %edi, %eax" << std::endl;
  ss << "subq $0x80000000, %rax" << std::endl;
  ss << "andq %rsi, %rax" << std::endl;
  ss << "movl %eax, %eax" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(1ul, result.second);  // Cost should be 1 for leakage
}

TEST_F(LeakageCostTest, AndlLeaks) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "movl %edi, %eax" << std::endl;
  ss << "andl %esi, %eax" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(1ul, result.second);  // Cost should be 1 for leakage
}

TEST_F(LeakageCostTest, AndqPartialTransformLeaks) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "movq %rsi, %rax" << std::endl;
  ss << "movq $0x8000000000000000, %r11" << std::endl;
  ss << "movw %di, %r11w" << std::endl;
  ss << "andq %rax, %r11" << std::endl;
  ss << "movw $1, %di" << std::endl;
  ss << "andq %rdi, %rax" << std::endl;
  ss << "movw %r11w, %ax" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(1ul, result.second);  // Cost should be 1 for leakage
}

TEST_F(LeakageCostTest, SublTransformZeroLeakyInstructions) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "movq %rcx, %r11" << std::endl;
  ss << "movl %ecx, %ecx" << std::endl;
  ss << "subq $0x80000000, %rcx" << std::endl;
  ss << "subq $0x80000000, %rcx" << std::endl;
  ss << "subq %rcx, %rax" << std::endl;
  ss << "movl %eax, %eax" << std::endl;
  ss << "movq %r11, %rcx" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  fxn_.set_reduction(LeakageReduction::N_INSTRUCTIONS);
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(0ul, result.second);  // Cost should be 0 for non-leaky transform
}

TEST_F(LeakageCostTest, SingleSubqOneLeakyInstruction) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "subq %rbx, %rax" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  fxn_.set_reduction(LeakageReduction::N_INSTRUCTIONS);
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(1ul, result.second);  // Cost should be 1 for leaky subq
}

TEST_F(LeakageCostTest, SubqAndqTwoLeakyInstructions) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a subq and an andq
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "subq %rbx, %rax" << std::endl;
  ss << "andq %rcx, %rax" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  fxn_.set_reduction(LeakageReduction::N_INSTRUCTIONS);
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(2ul, result.second);  // Cost should be 2 for leaky subq, andq
}

TEST_F(LeakageCostTest, SublTransformZeroEquivalenceClasses) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "movq %rcx, %r11" << std::endl;
  ss << "movl %ecx, %ecx" << std::endl;
  ss << "subq $0x80000000, %rcx" << std::endl;
  ss << "subq $0x80000000, %rcx" << std::endl;
  ss << "subq %rcx, %rax" << std::endl;
  ss << "movl %eax, %eax" << std::endl;
  ss << "movq %r11, %rcx" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  fxn_.set_reduction(LeakageReduction::N_EQUIVALENCE_CLASSES);
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(0ul, result.second);  // Cost should be 0 for non-leaky transform
}

TEST_F(LeakageCostTest, SingleSubqOneEquivalenceClass) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "subq %rbx, %rax" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  fxn_.set_reduction(LeakageReduction::N_EQUIVALENCE_CLASSES);
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(1ul, result.second);  // Cost should be 1 for leaky subq
}

TEST_F(LeakageCostTest, SublTransformZeroValueRanges) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "movq %rcx, %r11" << std::endl;
  ss << "movl %ecx, %ecx" << std::endl;
  ss << "subq $0x80000000, %rcx" << std::endl;
  ss << "subq $0x80000000, %rcx" << std::endl;
  ss << "subq %rcx, %rax" << std::endl;
  ss << "movl %eax, %eax" << std::endl;
  ss << "movq %r11, %rcx" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  fxn_.set_reduction(LeakageReduction::N_VALUE_RANGES);
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(0ul, result.second);  // Cost should be 0 for non-leaky transform
}

TEST_F(LeakageCostTest, SingleSubqTwoValueRanges) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "subq %rbx, %rax" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  fxn_.set_reduction(LeakageReduction::N_VALUE_RANGES);
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(2ul, result.second);  // Cost should be 1 for leaky subq
}

TEST_F(LeakageCostTest, SingleAddqThreeValueRanges) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases_for_zero(10);

  // Setup
  std::stringstream ss;
  x64asm::Code code;

  // Create a program with a single subq instruction
  ss.clear();
  ss << ".foo:" << std::endl;
  ss << "addq %rbx, %rax" << std::endl;
  ss << "retq" << std::endl;
  ss >> code;

  auto cfg = make_cfg(code);

  // Run the code to generate execution data for leakage analysis
  sb_.run(cfg);
  
  // Compute leakage cost
  fxn_.set_reduction(LeakageReduction::N_VALUE_RANGES);
  auto result = fxn_(cfg);

  // Expect the cost to be 1 (indicating leakage was detected)
  EXPECT_TRUE(result.first);  // Should be successful
  EXPECT_EQ(3ul, result.second);  // Cost should be 1 for leaky subq
}

} //namespace