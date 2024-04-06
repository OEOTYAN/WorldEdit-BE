#pragma once

#include "Region.h"
namespace we {
class PolyRegion : public Region {
public:
    std::vector<BlockPos> points;
    int                   minY = -2100000000;
    int                   maxY = -2100000000;
    BlockPos              mainPos{-2100000000, -2100000000, -2100000000};

    void updateBoundingBox() override;

    PolyRegion(const BoundingBox& region, int dim);

    bool expand(std::span<BlockPos>) override;

    bool contract(std::span<BlockPos>) override;

    bool shift(std::span<BlockPos>) override;

    uint64_t size() const override;

    void renderRegion() override;

    void forEachBlockUVInRegion(
        const std::function<void(BlockPos const&, double, double)>& todo
    ) override;

    Vec3 getCenter() const override {
        BlockPos tmp(0, 0, 0);
        for (auto& point : points) {
            tmp = tmp + point;
        }
        auto size = points.size();
        return Vec3(
            static_cast<double>(tmp.x) / size + 0.5,
            static_cast<double>(maxY + minY) * 0.5 + 0.5,
            static_cast<double>(tmp.z) / size + 0.5
        );
    };

    bool setMainPos(BlockPos const& pos, int dim) override;

    bool setVicePos(BlockPos const& pos, int dim) override;

    bool contains(BlockPos const& pos) override;
};
} // namespace we
