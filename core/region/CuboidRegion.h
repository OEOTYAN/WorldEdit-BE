//
// Created by OEOTYAN on 2021/2/8.
//
#pragma once

#include "Globals.h"
#include "Region.h"
namespace worldedit {
    class CuboidRegion final : public Region {
       public:
        BlockPos mainPos = BlockPos::MIN;
        BlockPos vicePos = BlockPos::MIN;
        void updateBoundingBox() override;

        std::string getName() override { return "cuboid"; }

        explicit CuboidRegion(const BoundingBox& region, const int& dim);
        explicit CuboidRegion();

        std::pair<std::string, bool> expand(
            const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> contract(
            const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> shift(const BlockPos& change) override;

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        void forEachLine(
            const std::function<void(const BlockPos&, const BlockPos&)>& todo)
            override;
    };
}  // namespace worldedit