#ifndef STOKE_SRC_STATE_LKG_H
#define STOKE_SRC_STATE_LKG_H

#include <cassert>
#include <iostream>
#include <array>

namespace stoke {

/** A leakage register. Conceptually, each leakage register represents
    side channel leakage for one "unit" of secret data, e.g., one register. 
*/
class Lkg {

public:
  static constexpr Lkg src() {
    return {};
  }

  static constexpr std::array<Lkg, 1> lkgs() {
    return {
      src()
    };
  }

  /** Comparison based on on val_. */
  constexpr bool operator<(const Lkg& rhs) const {
    return val_ < rhs.val_;
  }
  /** Comparison based on on val_. */
  constexpr bool operator==(const Lkg& rhs) const {
    return val_ == rhs.val_;
  }
  /** Comparison based on on val_. */
  constexpr bool operator!=(const Lkg& rhs) const {
    return !(*this == rhs);
  }

  /** Conversion based on underlying value. */
  constexpr operator uint64_t() const {
    return val_;
  }

protected:
  /** Creates a leakage register with specified underlying value. */
  constexpr Lkg(uint64_t val) : val_(val) {}
  /** Creates a leakage register with default underlying value */ 
  constexpr Lkg() : val_(0) {}

  /** Underlying value. */
  uint64_t val_;
};

}

#endif
