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

        std::pair<std::string, bool> expand(
            const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool>  contract(const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool>  shift(const BlockPos& change) override;

        int size() const override;

        void renderRegion() override;

        Vec3 getCenter() const override {
            BlockPos tmp(0, 0, 0);
            for (auto& point : points) {
                tmp = tmp + point;
            }
            return Vec3(static_cast<double>(tmp.x) / points.size() + 0.5,
                        static_cast<double>(maxY + minY) * 0.5 + 0.5,
                        static_cast<double>(tmp.z) / points.size() + 0.5);
        };

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        bool contains(const BlockPos& pos) override;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_POLYREGION_H