//
// Created by OEOTYAN on 2021/2/8.
//

#pragma once
#ifndef WORLDEDIT_REGION_H
#define WORLDEDIT_REGION_H
#include "pch.h"
#include "particle/Graphics.h"

namespace worldedit {
    inline AABB toRealAABB(const BoundingBox& bound) {
        return {{bound.bpos1.x, bound.bpos1.y, bound.bpos1.z},
                {bound.bpos2.x + 1, bound.bpos2.y + 1, bound.bpos2.z + 1}};
    }

    enum RegionType {
        CUBOID = 0,
        EXPAND = 1,
        SPHERE = 2,
        POLY = 3,
        CONVEX = 4,
        CYLINDER= 5,
    };

    class Region {
       protected:
        RegionType regionType = CUBOID;
        int rendertick = 0;
        int dimensionID = -1;
        BoundingBox boundingBox;

       public:
        bool selecting = false;
        bool needResetVice = true;
        explicit Region(const BoundingBox& b, int dim);

        BoundingBox getBoundBox() const { return this->boundingBox; }
        RegionType getRegionType() const { return this->regionType; }
        int getDimensionID() const { return this->dimensionID; }

        virtual ~Region() = default;

        virtual void updateBoundingBox() = 0;

        virtual int size() const {
            return (boundingBox.bpos2.x - boundingBox.bpos1.x + 1) *
                   (boundingBox.bpos2.y - boundingBox.bpos1.y + 1) *
                   (boundingBox.bpos2.z - boundingBox.bpos1.z + 1);
        };

        virtual std::pair<std::string, bool> expand(
            const std::vector<BlockPos>& changes) {
            return {"This region can not be extended", false};
        };

        virtual std::pair<std::string, bool> contract(
            const std::vector<BlockPos>& changes) {
            return {"This region can not be contracted", false};
        };
        virtual std::pair<std::string, bool> shift(const BlockPos& change) {
            return {"This region can not be shifted", false};
        };

        virtual Vec3 getCenter() const {
            return toRealAABB(boundingBox).getCenter();
        };

        virtual bool setMainPos(const BlockPos& pos, const int& dim) {
            return false;
        };

        virtual bool setVicePos(const BlockPos& pos, const int& dim) {
            return false;
        };

        virtual bool contains(const BlockPos& pos) {
            return pos.containedWithin(boundingBox.bpos1, boundingBox.bpos2);
        };

        virtual void forEachBlockInRegion(
            const std::function<void(const BlockPos&)>& todo);

        virtual void forTopBlockInRegion(
            const std::function<void(const BlockPos&)>& todo);

        virtual std::vector<double> getHeightMap();

        virtual void applyHeightMap(const std::vector<double>& data);

        virtual void renderRegion();

        inline bool hasSelected() { return this->selecting; }

        static Region* createRegion(RegionType type,
                                    const BoundingBox& box,
                                    int dim);
    };
}  // namespace worldedit
#endif  // WORLDEDIT_REGION_H