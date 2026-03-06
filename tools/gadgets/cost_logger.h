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

#ifndef STOKE_TOOLS_GADGETS_COST_LOGGER_H
#define STOKE_TOOLS_GADGETS_COST_LOGGER_H

#include <iostream>

#include "src/ext/cpputil/include/io/console.h"

#include "src/cost/cost_parser.h"
#include "src/cost/binsize.h"
#include "src/cost/measured.h"
#include "src/cost/size.h"
#include "src/cost/sseavx.h"
#include "src/cost/nongoal.h"
#include "tools/args/cost.inc"
#include "tools/gadgets/correctness_cost.h"
#include "tools/gadgets/latency_cost.h"
#include "tools/gadgets/leakage_cost.h"
#include "tools/gadgets/nongoal_cost.h"

using namespace std;

namespace stoke {

class CostLoggerGadget {
public:
  CostLoggerGadget(std::string file, const Cfg& target, Sandbox* test_sb, Sandbox* perf_sb) {
    log_ = file;
    ofstream outfile;
    outfile.open(log_);
    outfile << "name";

    CostParser::SymbolTable st;
    st["binsize"] =      new BinSizeCost();
    st["correctness"] =  new CorrectnessCostGadget(target, test_sb);
    st["latency"] =      new LatencyCostGadget();
    st["measured"] =     new MeasuredCost();
    st["size"] =         new SizeCost();
    st["sseavx"] =       new SseAvxCost();
    st["nongoal"] =      new NonGoalCostGadget(target);
    st["leakage"] =      new LeakageCostGadget();

    std::string s = cost_logger_arg.value();
    size_t index = 0;

    while (index != s.npos && index < s.length()) {
      size_t next = s.find(",", index);
      auto cost_arg = s.substr(index, next != s.npos ? next - index : s.npos);
      index = next != s.npos ? next + 1 : s.npos;

      CostParser cost_p(cost_arg, st);

      auto cost_fxn = cost_p.run();
      if (cost_p.get_error().size()) {
        cpputil::Console::error(1) << "Error parsing cost function \"" << cost_arg << "\": " << cost_p.get_error() << std::endl;
      }
      if (cost_fxn == NULL) {
        cpputil::Console::error(1) << "Unknown error parsing cost function \"" << cost_arg << "\"." << std::endl;
      }
      
      (*cost_fxn).setup_test_sandbox(test_sb).setup_perf_sandbox(perf_sb);
      fxns_.push_back(cost_fxn);
      outfile << "," << cost_arg;
    }

    outfile << endl;
    outfile.close();
  }

  void operator()(const std::string name, const Cfg& cfg, Cost max) {
    ofstream outfile;
    outfile.open(log_, std::ios_base::app);
    outfile << name;
    for (auto fxn : fxns_) {
      auto cost = (*fxn)(cfg, max);
      outfile << "," << to_string(cost.second);
    }
    outfile << endl;
    outfile.close();
  }

  void operator()(const std::string name, const Cfg& cfg) {
    ofstream outfile;
    outfile.open(log_, std::ios_base::app);
    outfile << name;
    for (auto fxn : fxns_) {
      auto cost = (*fxn)(cfg);
      outfile << "," << to_string(cost.second);
    }
    outfile << endl;
    outfile.close();
  }

private:

  std::vector<CostFunction*> fxns_;
  std::string log_;

};

} // namespace stoke

#endif
