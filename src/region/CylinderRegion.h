#pragma once

#include "Region.h"

namespace we {
class CylinderRegion : public Region {
    Pos2d  center;
    double radius;
    int    minY, maxY;

    GeoContainer circles;
    GeoContainer startbox;
    GeoContainer endbox;
    GeoContainer centerline;

    std::optional<int> checkChanges(std::span<BlockPos>) const;
    bool               setY(int y);

public:
    CylinderRegion(DimensionType, BoundingBox const&);

    ll::Expected<> serialize(CompoundTag&) const override;

    ll::Expected<> deserialize(CompoundTag const&) override;

    void updateBoundingBox() override;

    RegionType getType() const override { return RegionType::Cylinder; }

    size_t size() const override {
        return (size_t)std::round(std::numbers::pi * radius * radius * (maxY - minY + 1));
    };

    bool expand(std::span<BlockPos>) override;

    bool contract(std::span<BlockPos>) override;

    bool shift(BlockPos const&) override;

    ll::coro::Generator<std::tuple<BlockPos, double, double>>
    forEachBlockUVInRegion() const override;

    bool setMainPos(BlockPos const&) override;

    bool setOffPos(BlockPos const&) override;

    std::vector<std::string> getInfo() const override;

    bool contains(BlockPos const&) const override;
};
} // namespace we
