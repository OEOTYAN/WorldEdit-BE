#pragma once

#include "Region.h"
#include "utils/Bresenham.h"

namespace we {

class LoftRegion : public Region {
    std::vector<std::vector<BlockPos>> loftPoints;
    size_t                             maxPointCount = 1;

    std::vector<KochanekBartelsInterpolation> interpolations;
    std::vector<KochanekBartelsInterpolation> verticalCurves;
    bool                                      cycle = false;

    GeoContainer views;

    mutable phmap::flat_hash_map<BlockPos, std::pair<double, double>> posCache;
    mutable bool                                                      posCached = false;
    mutable BoundingBox                                               boundingBox;

    static constexpr int quality = 16;

    void buildCache() const;

public:
    LoftRegion(DimensionType, BoundingBox const&);

    ll::Expected<> serialize(CompoundTag&) const override;

    ll::Expected<> deserialize(CompoundTag const&) override;

    BoundingBox getBoundingBox() const override;

    void updateBoundingBox() override;

    RegionType getType() const override { return RegionType::Loft; }

    bool shift(BlockPos const& change) override;

    void setCycle(bool value) {
        cycle = value;
        updateBoundingBox();
    };

    uint64_t size() const override {
        auto bSize = boundingBox.max - boundingBox.min + 1;
        return std::max(
            std::max((uint64_t)bSize.x * bSize.z, (uint64_t)bSize.x * bSize.y),
            (uint64_t)bSize.y * bSize.z
        );
    }

    bool removePoint(std::optional<BlockPos> const&) override;

    void forEachBlockInRegion(std::function<void(BlockPos const&)>&&) const override;

    void forEachBlockUVInRegion(std::function<void(BlockPos const&, double, double)>&&)
        const override;

    void forEachBlockInLines(
        int                                         num,
        bool                                        isX,
        const std::function<void(BlockPos const&)>& todo
    );

    bool setMainPos(BlockPos const& pos) override;

    bool setOffPos(BlockPos const& pos) override;

    bool contains(BlockPos const& pos) const override;
};
} // namespace we
