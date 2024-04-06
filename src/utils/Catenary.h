#pragma once

#include "worldedit/WorldEdit.h"

namespace we {

inline double getCatenaryParameter(double d, double h, double l) {
    const double m = sqrt(l * l - h * h) / d;
    double       x = acosh(m) + 1;
    ll::meta::unroll<10>([&](size_t) { x -= (std::sinh(x) - m * x) / (std::cosh(x) - m); }
    );
    return d / (2 * x);
}
} // namespace we
