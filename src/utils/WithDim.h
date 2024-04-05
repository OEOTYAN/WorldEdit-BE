#pragma once

#include <mc/world/AutomaticID.h>

namespace we {
template <class T>
class WithDim {
    T             pos{};
    DimensionType dim{-1};
};
} // namespace we
