#ifndef STOKE_SRC_STATE_LKG_H
#define STOKE_SRC_STATE_LKG_H

#include <cassert>
#include <iostream>
#include <array>

namespace stoke {

/** A leakage register. Conceptually, each leakage register represents
    side channel leakage for one "unit" of secret data, e.g., one register. 
*/
class LeakageReg {

public:
  static constexpr LeakageReg srclkg() {
    return {};
  }

  static constexpr std::array<LeakageReg, 1> lkgs() {
    return {
      srclkg()
    };
  }

  /** Comparison based on on val_. */
  constexpr bool operator<(const LeakageReg& rhs) const {
    return val_ < rhs.val_;
  }
  /** Comparison based on on val_. */
  constexpr bool operator==(const LeakageReg& rhs) const {
    return val_ == rhs.val_;
  }
  /** Comparison based on on val_. */
  constexpr bool operator!=(const LeakageReg& rhs) const {
    return !(*this == rhs);
  }

  /** Conversion based on underlying value. */
  constexpr operator uint64_t() const {
    return val_;
  }

  /** Creates a leakage register with specified underlying value. */
  constexpr LeakageReg(uint64_t val) : val_(val) {}
  /** Creates a leakage register with default underlying value */ 
  constexpr LeakageReg() : val_(0) {}

protected:
  /** Underlying value. */
  uint64_t val_;
};

}

#endif
