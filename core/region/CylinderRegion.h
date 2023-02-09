//
// Created by OEOTYAN on 2022/06/04.
//
#pragma once
#include "Globals.h"
#include "Region.h"

namespace worldedit {
    class CylinderRegion : public Region {
       private:
        BlockPos center = BlockPos::MIN;
        float radius = 0.5;
        int minY = 0, maxY = 0;
        bool hasY = false, selectedCenter = false, selectedRadius = false;
        int checkChanges(const std::vector<BlockPos>& changes);

       public:
        std::string getName() override { return "cylinder"; }

        explicit CylinderRegion(const BoundingBox& region, const int& dim);

        void updateBoundingBox() override;

        bool setY(int y);

        uint64_t size() const override {
            return (uint64_t)std::round(M_PI * (double)radius * (double)radius *
                                        (maxY - minY + 1));
        };

        std::pair<std::string, bool> expand(
            const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> contract(
            const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> shift(const BlockPos& change) override;

        void forEachBlockUVInRegion(
            const std::function<void(const BlockPos&, double, double)>& todo)
            override;

        void renderRegion() override;

        float getRadius() const { return radius; };

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        bool contains(const BlockPos& pos) override;
    };
}  // namespace worldedit
