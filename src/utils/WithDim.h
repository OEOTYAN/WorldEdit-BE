#pragma once

#include <mc/deps/core/utility/AutomaticID.h>

namespace we {
template <class T>
class WithDim {
public:
    T             pos{};
    DimensionType dim{};

    T const* operator->() const { return &pos; }
    T*       operator->() { return &pos; }
    T const& operator*() const { return pos; }
    T&       operator*() { return pos; }
};
} // namespace we
