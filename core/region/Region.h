//
// Created by OEOTYAN on 2021/2/8.
//

#pragma once
#ifndef WORLDEDIT_REGION_H
#define WORLDEDIT_REGION_H
#include "Global.h"
#include "particle/Graphics.h"

namespace worldedit {
    inline AABB toRealAABB(const BoundingBox& bound) {
        return {{bound.min.x, bound.min.y, bound.min.z}, {bound.max.x + 1, bound.max.y + 1, bound.max.z + 1}};
    }

    enum RegionType {
        CUBOID = 0,
        EXPAND = 1,
        SPHERE = 2,
        POLY = 3,
        CONVEX = 4,
        CYLINDER = 5,
        LOFT = 6,
    };

    class Region {
       protected:
        int rendertick = 0;
        int dimensionID = -1;
        BoundingBox boundingBox;

       public:
        RegionType regionType = CUBOID;
        bool selecting = false;
        bool needResetVice = true;
        explicit Region(const BoundingBox& b, int dim);

        virtual std::string getName() { return "null"; }
        virtual BoundingBox getBoundBox() { return this->boundingBox; }
        RegionType getRegionType() const { return this->regionType; }
        int getDimensionID() const { return this->dimensionID; }

        virtual ~Region() = default;

        virtual void updateBoundingBox() = 0;

        virtual uint64_t size() const {
            return (uint64_t)(boundingBox.max.x - boundingBox.min.x + 1) * (boundingBox.max.y - boundingBox.min.y + 1) *
                   (boundingBox.max.z - boundingBox.min.z + 1);
        };

        virtual std::pair<std::string, bool> expand(const std::vector<BlockPos>& changes) {
            return {"worldedit.selection.expand.error", false};
        };

        virtual std::pair<std::string, bool> contract(const std::vector<BlockPos>& changes) {
            return {"worldedit.selection.contract.error", false};
        };
        virtual std::pair<std::string, bool> shift(const BlockPos& change) {
            return {"worldedit.selection.shift.error", false};
        };

        virtual Vec3 getCenter() const { return toRealAABB(boundingBox).getCenter(); };

        virtual bool setMainPos(const BlockPos& pos, const int& dim) { return false; };

        virtual bool setVicePos(const BlockPos& pos, const int& dim) { return false; };

        virtual bool contains(const BlockPos& pos) { return pos.containedWithin(boundingBox.min, boundingBox.max); };

        virtual void forEachBlockInRegion(const std::function<void(const BlockPos&)>& todo);

        virtual void forEachBlockUVInRegion(const std::function<void(const BlockPos&, double, double)>& todo);

        virtual void forTopBlockInRegion(const std::function<void(const BlockPos&)>& todo);

        virtual void forEachLine(const std::function<void(const BlockPos&, const BlockPos&)>& todo){};

        virtual std::vector<double> getHeightMap(std::string mask = "");

        virtual bool removePoint(int dim, const BlockPos& pos = BlockPos::MIN) { return false; };

        virtual void applyHeightMap(const std::vector<double>& data, std::string xuid, std::string mask = "");

        virtual void renderRegion();

        int getHeighest(int x, int z);

        inline bool hasSelected() { return this->selecting; }

        static Region* createRegion(RegionType type, const BoundingBox& box, int dim);
    };
}  // namespace worldedit
#endif  // WORLDEDIT_REGION_H