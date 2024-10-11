#pragma once

#include <mc/deps/core/utility/AutomaticID.h>

namespace we {
template <class T>
class WithDim {
public:
    T             pos{};
    DimensionType dim{};
};
} // namespace we
