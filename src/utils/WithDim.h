#pragma once

#include <mc/world/AutomaticID.h>

namespace we {
template <class T>
class WithDim {
public:
    T             pos{};
    DimensionType dim{};
};
} // namespace we
