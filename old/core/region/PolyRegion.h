//
// Created by OEOTYAN on 2021/2/8.
//
#pragma once

#include "Globals.h"
#include "Region.h"
namespace worldedit {
    class PolyRegion final : public Region {
       public:
        std::vector<BlockPos> points;
        int minY = -2100000000;
        int maxY = -2100000000;
        BlockPos mainPos{-2100000000, -2100000000, -2100000000};

        std::string getName() override { return "poly"; }

        void updateBoundingBox() override;

        explicit PolyRegion(const BoundingBox& region, const int& dim);

        std::pair<std::string, bool> expand(const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> contract(const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> shift(const BlockPos& change) override;

        uint64_t size() const override;

        void renderRegion() override;

        void forEachBlockUVInRegion(const std::function<void(const BlockPos&, double, double)>& todo) override;

        Vec3 getCenter() const override {
            BlockPos tmp(0, 0, 0);
            for (auto& point : points) {
                tmp = tmp + point;
            }
            auto size = points.size();
            return Vec3(static_cast<double>(tmp.x) / size + 0.5, static_cast<double>(maxY + minY) * 0.5 + 0.5,
                        static_cast<double>(tmp.z) / size + 0.5);
        };

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        bool contains(const BlockPos& pos) override;
    };
}  // namespace worldedit
