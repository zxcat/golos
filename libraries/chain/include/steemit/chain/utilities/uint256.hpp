#pragma once

#include <steemit/protocol/types.hpp>

#include <fc/uint128_t.hpp>

namespace steemit {
    namespace chain {
        namespace utilities {
            inline u256 to256(const fc::uint128_t &t) {
                u256 v(t.hi);
                v <<= 64;
                v += t.lo;
                return v;
            }
        }
    }
}
