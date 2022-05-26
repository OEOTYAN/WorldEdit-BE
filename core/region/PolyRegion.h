//
// Created by OEOTYAN on 2021/2/8.
//
#pragma once
#ifndef WORLDEDIT_POLYREGION_H
#define WORLDEDIT_POLYREGION_H

#include "pch.h"
#include "Region.h"
namespace worldedit {
    class PolyRegion : public Region {
       public:
        std::vector<BlockPos> points;
        int minY = -2100000000;
        int maxY = -2100000000;
        BlockPos mainPos{-2100000000, -2100000000, -2100000000};

        void updateBoundingBox() override;

        explicit PolyRegion(const BoundingBox& region, const int& dim);

        bool expand(const std::vector<BlockPos>& changes,
                    Player* player) override;

        bool contract(const std::vector<BlockPos>& changes,
                      Player* player) override;

        bool expand(const BlockPos& change, Player* player) override;

        bool contract(const BlockPos& change, Player* player) override;

        bool shift(const BlockPos& change, Player* player) override;

        int size() const override;

        void renderRegion() override;

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        bool contains(const BlockPos& pos) override;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_POLYREGION_H