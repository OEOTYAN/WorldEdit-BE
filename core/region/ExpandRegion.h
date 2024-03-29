//
// Created by OEOTYAN on 2021/2/8.
//
#pragma once

#include "Globals.h"
#include "Region.h"
namespace worldedit {
    class ExpandRegion final : public Region {
       public:
        BlockPos mainPos = BlockPos::MIN;
        BlockPos vicePos = BlockPos::MIN;
        void updateBoundingBox() override;

        std::string getName() override { return "extend"; }

        explicit ExpandRegion(const BoundingBox& region, const int& dim);

        std::pair<std::string, bool> expand(const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> contract(const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> shift(const BlockPos& change) override;

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;
    };
}  // namespace worldedit