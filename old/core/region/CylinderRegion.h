#pragma once

#include "Region.h"

namespace we {
class CylinderRegion : public Region {
private:
    BlockPos center = BlockPos::MIN;
    float    radius = 0.5;
    int      minY = 0, maxY = 0;
    bool     hasY = false, selectedCenter = false, selectedRadius = false;
    int      checkChanges(const std::vector<BlockPos>& changes);

public:
    CylinderRegion(const BoundingBox& region, int dim);

    void updateBoundingBox() override;

    bool setY(int y);

    uint64_t size() const override {
        return (uint64_t
        )std::round(M_PI * (double)radius * (double)radius * (maxY - minY + 1));
    };

    bool expand(std::span<BlockPos>) override;

    bool contract(std::span<BlockPos>) override;

    bool shift(std::span<BlockPos>) override;

    void forEachBlockUVInRegion(
        const std::function<void(BlockPos const&, double, double)>& todo
    ) const override;

    void renderRegion() override;

    float getRadius() const { return radius; };

    bool setMainPos(BlockPos const& pos, int dim) override;

    bool setVicePos(BlockPos const& pos, int dim) override;

    bool contains(BlockPos const& pos) override;
};
} // namespace we
