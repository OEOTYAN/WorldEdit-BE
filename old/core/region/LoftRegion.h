//
// Created by OEOTYAN on 2022/06/18.
//
#pragma once

#include "Globals.h"
#include "Region.h"
#include "eval/Bresenham.hpp"

namespace worldedit {

    class LoftRegion final : public Region {
       public:
        std::vector<std::vector<BlockPos>> loftPoints;
        int maxPointCount = 1;
        std::vector<KochanekBartelsInterpolation> interpolations;
        std::vector<KochanekBartelsInterpolation> cache1;
        std::vector<KochanekBartelsInterpolation> cache2;
        phmap::flat_hash_map<BlockPos, std::pair<double, double>> posCache;
        bool circle = false;
        static const int quality = 16;
        bool posCached = false;

        std::string getName() override { return "loft"; }

        void updateBoundingBox() override;

        explicit LoftRegion(const BoundingBox& region, const int& dim);

        BoundingBox getBoundBox() override;

        std::pair<std::string, bool> shift(const BlockPos& change) override;

        void setCircle(bool value) {
            circle = value;
            updateBoundingBox();
        };

        void buildCache();

        uint64_t size() const override {
            auto bSize = boundingBox.max - boundingBox.min + 1;
            return std::max(std::max((uint64_t)bSize.x * bSize.z, (uint64_t)bSize.x * bSize.y),
                            (uint64_t)bSize.y * bSize.z);
        }

        bool removePoint(int dim, const BlockPos& pos = BlockPos::MIN) override;

        void forEachBlockInRegion(const std::function<void(const BlockPos&)>& todo) override;

        void forEachBlockUVInRegion(const std::function<void(const BlockPos&, double, double)>& todo) override;

        void forEachBlockInLines(int num, bool isX, const std::function<void(const BlockPos&)>& todo);

        void renderRegion() override;

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        bool contains(const BlockPos& pos);
    };
}  // namespace worldedit
