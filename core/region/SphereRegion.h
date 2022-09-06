//
// Created by OEOTYAN on 2021/2/8.
//

#ifndef WORLDEDIT_SPHEREREGION_H
#define WORLDEDIT_SPHEREREGION_H

#include "Global.h"
#include "Region.h"

namespace worldedit {
    class SphereRegion : public Region {
       private:
        BlockPos center = BlockPos::MIN;
        double radius = 0.5;
        int checkChanges(const std::vector<BlockPos>& changes);

       public:
        void updateBoundingBox() override;

        explicit SphereRegion(const BoundingBox& region, const int& dim);

        std::pair<std::string, bool> expand(const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> contract(const std::vector<BlockPos>& changes) override;

        std::pair<std::string, bool> shift(const BlockPos& change) override;

        Vec3 getCenter() const override { return center.toVec3() + 0.5f; };

        int size() const override {
            return (int)std::round(4.0 / 3.0 * M_PI * radius * radius * radius);
        };

        void forEachBlockUVInRegion(const std::function<void(const BlockPos&, double, double)>& todo) override;

        void renderRegion() override;

        double getRadius() const { return radius; };

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        bool contains(const BlockPos& pos) override;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_SPHEREREGION_H