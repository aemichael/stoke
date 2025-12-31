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

#include <array>
#include <string>
#include <utility>

#include "tools/io/generic.h"
#include "tools/io/leakage_reduction.h"

using namespace std;
using namespace stoke;

namespace {

array<pair<string, LeakageReduction>, 4> rs {{
    {"binary", LeakageReduction::BINARY},
    {"instructions", LeakageReduction::N_INSTRUCTIONS},
    {"equivalence_classes", LeakageReduction::N_EQUIVALENCE_CLASSES},
    {"value_ranges", LeakageReduction::N_VALUE_RANGES},
  }
};

} // namespace

namespace stoke {

void LeakageReductionReader::operator()(std::istream& is, LeakageReduction& r) {
  string s;
  is >> s;
  if (!generic_read(rs, s, r)) {
    is.setstate(ios::failbit);
  }
}

void LeakageReductionWriter::operator()(std::ostream& os, const LeakageReduction r) {
  string s;
  generic_write(rs, s, r);
  os << s;
}

} // namespace stoke
