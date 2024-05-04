#pragma once

#include "Region.h"

namespace we {
class PolyRegion : public Region {
    std::vector<Pos2d> points;
    int                minY;
    int                maxY;
    GeoContainer       outline;

public:
    PolyRegion(DimensionType, BoundingBox const&);

    ll::Expected<> serialize(CompoundTag&) const override;

    ll::Expected<> deserialize(CompoundTag const&) override;

    void updateBoundingBox() override;

    bool expand(std::span<BlockPos>) override;

    bool contract(std::span<BlockPos>) override;

    bool shift(BlockPos const&) override;

    RegionType getType() const override { return RegionType::Poly; }

    uint64 size() const override;

    bool removePoint(std::optional<BlockPos> const&) override;

    void
    forEachBlockUVInRegion(std::function<void(BlockPos const&, double, double)>&& todo
    ) const override;

    Vec3 getCenter() const override;

    bool setMainPos(BlockPos const&) override;

    bool setOffPos(BlockPos const&) override;

    bool contains(BlockPos const&) const override;
};
} // namespace we
