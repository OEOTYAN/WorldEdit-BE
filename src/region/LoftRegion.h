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

    mutable ll::DenseMap<BlockPos, std::pair<double, double>> posCache;
    mutable bool                                              posCached = false;
    mutable BoundingBox                                       boundingBox;

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

    std::vector<std::string> getInfo() const override;

    size_t size() const override {
        auto bSize = boundingBox.max - boundingBox.min + 1;
        return std::max(
            std::max((size_t)bSize.x * bSize.z, (size_t)bSize.x * bSize.y),
            (size_t)bSize.y * bSize.z
        );
    }

    bool removePoint(std::optional<BlockPos> const&) override;

    ll::coro::Generator<BlockPos> forEachBlockInRegion() const override;

    ll::coro::Generator<std::tuple<BlockPos, double, double>>
    forEachBlockUVInRegion() const override;

    ll::coro::Generator<BlockPos> forEachBlockInLines(int num, bool isX) const;

    ll::coro::Generator<std::pair<BlockPos, BlockPos>> forEachLine() const override;

    bool setMainPos(BlockPos const& pos) override;

    bool setOffPos(BlockPos const& pos) override;

    bool contains(BlockPos const& pos) const override;
};
} // namespace we
