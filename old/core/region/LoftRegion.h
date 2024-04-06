#pragma once

#include "Region.h"
#include "math/Bresenham.h"

namespace we {

class LoftRegion : public Region {
public:
    std::vector<std::vector<BlockPos>>                        loftPoints;
    int                                                       maxPointCount = 1;
    std::vector<KochanekBartelsInterpolation>                 interpolations;
    std::vector<KochanekBartelsInterpolation>                 cache1;
    std::vector<KochanekBartelsInterpolation>                 cache2;
    phmap::flat_hash_map<BlockPos, std::pair<double, double>> posCache;
    bool                                                      circle    = false;
    static const int                                          quality   = 16;
    bool                                                      posCached = false;

    void updateBoundingBox() override;

    LoftRegion(const BoundingBox& region, int dim);

    BoundingBox getBoundBox() override;

    bool shift(BlockPos const& change) override;

    void setCircle(bool value) {
        circle = value;
        updateBoundingBox();
    };

    void buildCache();

    uint64_t size() const override {
        auto bSize = boundingBox.max - boundingBox.min + 1;
        return std::max(
            std::max((uint64_t)bSize.x * bSize.z, (uint64_t)bSize.x * bSize.y),
            (uint64_t)bSize.y * bSize.z
        );
    }

    bool removePoint(int dim, BlockPos const& pos = BlockPos::MIN) override;

    void forEachBlockInRegion(const std::function<void(BlockPos const&)>& todo) override;

    void forEachBlockUVInRegion(
        const std::function<void(BlockPos const&, double, double)>& todo
    ) override;

    void forEachBlockInLines(
        int                                         num,
        bool                                        isX,
        const std::function<void(BlockPos const&)>& todo
    );

    void renderRegion() override;

    bool setMainPos(BlockPos const& pos, int dim) override;

    bool setVicePos(BlockPos const& pos, int dim) override;

    bool contains(BlockPos const& pos);
};
} // namespace we
