//
// Created by OEOTYAN on 2022/06/18.
//
#pragma once
#ifndef WORLDEDIT_LOFTREGION_H
#define WORLDEDIT_LOFTREGION_H

#include "pch.h"
#include "Region.h"
#include "eval/Bresenham.hpp"

namespace worldedit {

    class PosWithUV {
       public:
        int x, y, z;
        double u, v;
        PosWithUV(int x, int y, int z, double u, double v) : x(x), y(y), z(z), u(u), v(v) {}
        PosWithUV(BlockPos const& pos, double u, double v) : x(pos.x), y(pos.y), z(pos.z), u(u), v(v) {}
        BlockPos getBlockPos() const { return BlockPos(x, y, z); }
        bool inline operator==(PosWithUV const& other) const { return x == other.x && y == other.y && z == other.z; }
    };

    class _hash2 {
       public:
        size_t operator()(const PosWithUV& rc) const { return std::hash<BlockPos>()(rc.getBlockPos()); }
    };

    class LoftRegion : public Region {
       public:
        std::vector<std::vector<BlockPos>> loftPoints;
        int maxPointCount = 1;
        std::vector<KochanekBartelsInterpolation> interpolations;
        std::vector<KochanekBartelsInterpolation> cache1;
        std::vector<KochanekBartelsInterpolation> cache2;
        std::unordered_set<PosWithUV, _hash2> posCache;
        bool circle = false;
        static const int quality = 16;
        bool posCached = false;

        void updateBoundingBox() override;

        explicit LoftRegion(const BoundingBox& region, const int& dim);

        BoundingBox getBoundBox() override;

        std::pair<std::string, bool> shift(const BlockPos& change) override;

        void setCircle(bool value) {
            circle = value;
            updateBoundingBox();
        };

        void buildCache();

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

#endif  // WORLDEDIT_LOFTREGION_H