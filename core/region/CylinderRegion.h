//
// Created by OEOTYAN on 2022/06/04.
//
#pragma once
#ifndef WORLDEDIT_CYLINDERREGION_H
#define WORLDEDIT_CYLINDERREGION_H
#include "pch.h"
#include "Region.h"
#ifndef __M__PI__
#define __M__PI__ 3.141592653589793238462643383279
#endif

namespace worldedit {
    class CylinderRegion : public Region {
       private:
        BlockPos center = BlockPos::MIN;
        float    radius = 0.5;
        int      minY = 0, maxY = 0;
        bool     hasY = false, selectedCenter = false, selectedRadius = false;
        int      checkChanges(const std::vector<BlockPos>& changes);

       public:
        explicit CylinderRegion(const BoundingBox& region, const int& dim);

        void updateBoundingBox() override;

        bool setY(int y);

        int size() const override {
            return (int)std::round(__M__PI__ * (double)radius * (double)radius * (maxY - minY + 1));
        };

        std::pair<std::string, bool> expand(const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> contract(const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> shift(const BlockPos& change) override;

        void renderRegion() override;

        float getRadius() const { return radius; };

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        bool contains(const BlockPos& pos) override;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_CYLINDERREGION_H