#pragma once

#include <mc/common/FacingID.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/world/level/BlockPos.h>

namespace we {
template <class Vec>
    requires(Vec::size() == 3)
constexpr FacingID dirToFacing(Vec const& v) {
    auto absed = abs(v);
    if (auto m = std::max({absed.x, absed.y, absed.z}); m == absed.y) {
        return v.y < 0 ? FacingID::Down : FacingID::Up;
    } else if (m == absed.x) {
        return v.x < 0 ? FacingID::West : FacingID::East;
    } else {
        return v.z < 0 ? FacingID::North : FacingID::South;
    }
}
template <class Vec = BlockPos, class Num = int>
    requires(Vec::size() == 3)
constexpr Vec facingToDir(FacingID facing, Num length = 1) {
    switch (facing) {
    case FacingID::Down:
        return Vec{0, -length, 0};
    case FacingID::Up:
        return Vec{0, length, 0};
    case FacingID::North:
        return Vec{0, 0, -length};
    case FacingID::South:
        return Vec{0, 0, length};
    case FacingID::West:
        return Vec{-length, 0, 0};
    case FacingID::East:
        return Vec{length, 0, 0};
    default:
        return Vec{0, 0, 0};
    }
}
constexpr FacingID opposite(FacingID facing) {
    switch (facing) {
    case FacingID::Down:
        return FacingID::Up;
    case FacingID::Up:
        return FacingID::Down;
    case FacingID::North:
        return FacingID::South;
    case FacingID::South:
        return FacingID::North;
    case FacingID::West:
        return FacingID::East;
    case FacingID::East:
        return FacingID::West;
    default:
        return FacingID::Unknown;
    }
}
} // namespace we
