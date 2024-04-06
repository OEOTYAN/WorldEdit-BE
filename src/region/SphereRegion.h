#pragma once

#include "Region.h"

namespace we {
class SphereRegion : public Region {
    GeoContainer sphere;
    GeoContainer centerbox;

    BlockPos center;
    double   radius;

    static std::optional<int> checkChanges(std::span<BlockPos>);

public:
    SphereRegion(DimensionType, BoundingBox const&);

    virtual void serialize(CompoundTag&) const;

    virtual void deserialize(CompoundTag const&);

    void updateBoundingBox() override;

    Type getType() const override { return Sphere; }

    bool expand(std::span<BlockPos>) override;

    bool contract(std::span<BlockPos>) override;

    bool shift(BlockPos const&) override;

    Vec3 getCenter() const override { return center.center(); };

    uint64_t size() const override {
        return (uint64_t
        )std::round((4.0 / 3.0 * std::numbers::pi) * radius * radius * radius);
    };

    void forEachBlockUVInRegion(std::function<void(BlockPos const&, double, double)>&&)
        const override;

    bool setMainPos(BlockPos const&) override;

    bool setVicePos(BlockPos const&) override;

    bool contains(BlockPos const& pos) const override {
        return pos.distanceTo(center) <= radius;
    }
};
} // namespace we
