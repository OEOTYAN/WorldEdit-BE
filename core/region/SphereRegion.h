//
// Created by OEOTYAN on 2021/2/8.
//

#ifndef WORLDEDIT_SPHEREREGION_H
#define WORLDEDIT_SPHEREREGION_H

#include "Region.h"
#include <cmath>
#ifndef __M__PI__
#define __M__PI__ 3.141592653589793238462643383279
#endif
namespace worldedit {
    class SphereRegion : public Region {
       private:
        BlockPos center = {0, -65, 0};
        float radius = 0.5;
        int checkChanges(const std::vector<BlockPos>& changes);

       public:
        void updateBoundingBox() override;

        explicit SphereRegion(const BoundingBox& region, const int& dim);

        bool expand(const std::vector<BlockPos>& changes,
                    Player* player) override;

        bool contract(const std::vector<BlockPos>& changes,
                      Player* player) override;

        bool expand(const BlockPos& change, Player* player) override;

        bool contract(const BlockPos& change, Player* player) override;

        bool shift(const BlockPos& change, Player* player) override;

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        Vec3 getCenter() const override {
            return center.toVec3() + Vec3(0.5f, 0.5f, 0.5f);
        };

        int size() const override {
            return (int)std::round(4.0 / 3.0 * __M__PI__ * (double)radius *
                                   (double)radius * (double)radius);
        };

        void renderRegion() override;

        float getRadius() const { return radius; };

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        bool contains(const BlockPos& pos) override;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_SPHEREREGION_H