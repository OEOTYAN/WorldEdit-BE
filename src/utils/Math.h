#pragma once

#include "worldedit/Global.h"

#include <ll/api/utils/RandomUtils.h>

namespace we {

inline double posfmod(double x, double y) { return x - std::floor(x / y) * y; }

template <class T>
constexpr T binpow(T const& a, uint64_t const& b) {
    if (b == 0) return static_cast<T>(1);
    T res = binpow(a, b / 2);
    if (b % 2) return res * res * a;
    else return res * res;
}

template <class T>
constexpr T pow2(T const& a) {
    return binpow<T>(a, 2);
}

template <class T>
inline T saturate(T const& a) {
    return std::min(static_cast<T>(1), std::max(a, static_cast<T>(0)));
}

template <class T>
inline T sign(T const& a) {
    if (a == 0) {
        return 0;
    }
    return std::copysign(1, a);
}

template <class T>
inline T signedSqrt(T const& a) {
    return sign(a) * std::sqrt(std::abs(a));
}

void plotLine(
    BlockPos const&                             pos0,
    BlockPos const&                             pos1,
    std::function<void(BlockPos const&)> const& todo
);
} // namespace we
