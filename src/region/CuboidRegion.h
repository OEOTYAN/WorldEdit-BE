#pragma once

#include "Region.h"

namespace we {
class CuboidRegion : public Region {
    GeoContainer box;

protected:
    BlockPos mainPos;
    BlockPos offPos;

public:
    CuboidRegion(DimensionType, BoundingBox const&);

    ll::Expected<> serialize(CompoundTag&) const override;

    ll::Expected<> deserialize(CompoundTag const&) override;

    void updateBoundingBox() override;

    RegionType getType() const override { return RegionType::Cuboid; }

    bool needResetVice() const override { return false; }

    bool expand(std::span<BlockPos>) override;

    bool contract(std::span<BlockPos>) override;

    bool shift(BlockPos const&) override;

    bool setMainPos(BlockPos const&) override;

    bool setOffPos(BlockPos const&) override;

    std::vector<std::string> getInfo() const override;

    ll::coro::Generator<std::pair<BlockPos, BlockPos>> forEachLine() const override;
};
} // namespace we
