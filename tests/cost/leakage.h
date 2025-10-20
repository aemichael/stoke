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

  void add_testcases(int count) {
    for (int i = 0; i < count; ++i) {
      auto state = get_state();
      
      // Set rax register to 0 when i == 0
      if (i == 0) {
        state.gp[x64asm::rax].get_fixed_quad(0) = 0;
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

TEST_F(LeakageCostTest, SingleSubqReturnsOne) {

  // Add testcases with different values to trigger leakage patterns
  add_testcases(10);

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

} //namespace