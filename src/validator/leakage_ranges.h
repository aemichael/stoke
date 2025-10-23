// Auto-generated leakage range mappings
// Generated from leakage semantics

#ifndef STOKE_LEAKAGE_RANGES_H
#define STOKE_LEAKAGE_RANGES_H

#include <unordered_map>
#include <vector>
#include <tuple>
#include "src/ext/x64asm/include/x64asm.h"

namespace std {
template<>
struct hash<x64asm::Opcode> {
  std::size_t operator()(const x64asm::Opcode& opcode) const {
    return std::hash<int32_t>()(static_cast<int32_t>(opcode));
  }
};
}

namespace stoke {

// Partition: a list of (low, high) value ranges
struct Partition {
    std::vector<std::tuple<int, int>> ranges;
};

// Map from opcode to pair of vectors: first vector for operand 0 (R1/src), second for operand 1 (R2/dst)
// Each vector contains partitions, where each partition represents ranges from one equivalence class
std::unordered_map<x64asm::Opcode, std::tuple<std::vector<Partition>, std::vector<Partition>>> leakage_ranges = {
    {x64asm::NEG_R16, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 65535)}}}, std::vector<Partition>{})},
    {x64asm::SHL_R32_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 4294967295)}}})},
    {x64asm::SAR_R64_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(18446744073709551615, 18446744073709551615)}}, Partition{{std::make_tuple(1, 18446744073709551614)}}})},
    {x64asm::OR_R16_R16, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(65535, 65535)}}, Partition{{std::make_tuple(1, 65534)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 65535)}}, Partition{{std::make_tuple(0, 65535)}}})},
    {x64asm::SHR_R32_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 4294967295)}}})},
    {x64asm::ADD_R16_R16, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 65535)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 65535)}}, Partition{{std::make_tuple(0, 65535)}}})},
    {x64asm::ADD_R32_R32, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 4294967295)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 4294967295)}}, Partition{{std::make_tuple(0, 4294967295)}}})},
    {x64asm::OR_R32_R32, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(4294967295, 4294967295)}}, Partition{{std::make_tuple(1, 4294967294)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 4294967295)}}, Partition{{std::make_tuple(0, 4294967295)}}})},
    {x64asm::OR_R8_R8, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(255, 255)}}, Partition{{std::make_tuple(1, 254)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 255)}}, Partition{{std::make_tuple(0, 255)}}})},
    {x64asm::AND_R16_R16, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(65535, 65535)}}, Partition{{std::make_tuple(1, 65534)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 65535)}}, Partition{{std::make_tuple(0, 65535)}}})},
    {x64asm::SHL_R8_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 255)}}})},
    {x64asm::SHR_R16_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 65535)}}})},
    {x64asm::NEG_R64, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 18446744073709551615)}}}, std::vector<Partition>{})},
    {x64asm::XOR_R8_R8, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(255, 255)}}, Partition{{std::make_tuple(1, 254)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 255)}}, Partition{{std::make_tuple(0, 255)}}})},
    {x64asm::AND_R32_R32, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(4294967295, 4294967295)}}, Partition{{std::make_tuple(1, 4294967294)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 4294967295)}}, Partition{{std::make_tuple(0, 4294967295)}}})},
    {x64asm::XOR_R64_R64, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(18446744073709551615, 18446744073709551615)}}, Partition{{std::make_tuple(1, 18446744073709551614)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 18446744073709551615)}}, Partition{{std::make_tuple(0, 18446744073709551615)}}})},
    {x64asm::SUB_R32_R32, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 4294967295)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 4294967295)}}, Partition{{std::make_tuple(0, 4294967295)}}})},
    {x64asm::SAR_R16_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(65535, 65535)}}, Partition{{std::make_tuple(1, 65534)}}})},
    {x64asm::SHL_R16_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 65535)}}})},
    {x64asm::SHR_R8_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 255)}}})},
    {x64asm::SAR_R8_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(255, 255)}}, Partition{{std::make_tuple(1, 254)}}})},
    {x64asm::SUB_R16_R16, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 65535)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 65535)}}, Partition{{std::make_tuple(0, 65535)}}})},
    {x64asm::SHL_R16_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 65535)}}})},
    {x64asm::SHR_R16_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 65535)}}})},
    {x64asm::SHL_R64_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 18446744073709551615)}}})},
    {x64asm::ADD_R8_R8, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 255)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 255)}}, Partition{{std::make_tuple(0, 255)}}})},
    {x64asm::SHR_R64_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 18446744073709551615)}}})},
    {x64asm::SHR_R64_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 18446744073709551615)}}})},
    {x64asm::SAR_R64_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(18446744073709551615, 18446744073709551615)}}, Partition{{std::make_tuple(1, 18446744073709551614)}}})},
    {x64asm::SHL_R64_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 18446744073709551615)}}})},
    {x64asm::SAR_R32_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(4294967295, 4294967295)}}, Partition{{std::make_tuple(1, 4294967294)}}})},
    {x64asm::SHR_R32_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 4294967295)}}})},
    {x64asm::SHL_R32_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 4294967295)}}})},
    {x64asm::OR_R64_R64, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(18446744073709551615, 18446744073709551615)}}, Partition{{std::make_tuple(1, 18446744073709551614)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 18446744073709551615)}}, Partition{{std::make_tuple(0, 18446744073709551615)}}})},
    {x64asm::ADD_R64_R64, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 18446744073709551615)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 18446744073709551615)}}, Partition{{std::make_tuple(0, 18446744073709551615)}}})},
    {x64asm::SAR_R32_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(4294967295, 4294967295)}}, Partition{{std::make_tuple(1, 4294967294)}}})},
    {x64asm::SAR_R8_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(255, 255)}}, Partition{{std::make_tuple(1, 254)}}})},
    {x64asm::SHL_R8_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 255)}}})},
    {x64asm::AND_R64_R64, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(18446744073709551615, 18446744073709551615)}}, Partition{{std::make_tuple(1, 18446744073709551614)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 18446744073709551615)}}, Partition{{std::make_tuple(0, 18446744073709551615)}}})},
    {x64asm::SAR_R16_CL, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(65535, 65535)}}, Partition{{std::make_tuple(1, 65534)}}})},
    {x64asm::NEG_R32, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 4294967295)}}}, std::vector<Partition>{})},
    {x64asm::NEG_R8, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 255)}}}, std::vector<Partition>{})},
    {x64asm::XOR_R16_R16, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(65535, 65535)}}, Partition{{std::make_tuple(1, 65534)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 65535)}}, Partition{{std::make_tuple(0, 65535)}}})},
    {x64asm::SUB_R64_R64, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 18446744073709551615)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 18446744073709551615)}}, Partition{{std::make_tuple(0, 18446744073709551615)}}})},
    {x64asm::SHR_R8_ONE, std::make_tuple(std::vector<Partition>{}, std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 255)}}})},
    {x64asm::XOR_R32_R32, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(4294967295, 4294967295)}}, Partition{{std::make_tuple(1, 4294967294)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 4294967295)}}, Partition{{std::make_tuple(0, 4294967295)}}})},
    {x64asm::AND_R8_R8, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0), std::make_tuple(255, 255)}}, Partition{{std::make_tuple(1, 254)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 255)}}, Partition{{std::make_tuple(0, 255)}}})},
    {x64asm::SUB_R8_R8, std::make_tuple(std::vector<Partition>{Partition{{std::make_tuple(0, 0)}}, Partition{{std::make_tuple(1, 255)}}}, std::vector<Partition>{Partition{{std::make_tuple(0, 255)}}, Partition{{std::make_tuple(0, 255)}}})}
};

} // namespace stoke

#endif // STOKE_LEAKAGE_RANGES_H
