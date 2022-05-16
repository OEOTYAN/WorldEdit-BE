//
// Created by OEOTYAN on 2021/2/8.
//

#ifndef WORLDEDIT_CUBOIDREGION_H
#define WORLDEDIT_CUBOIDREGION_H

#include "Region.h"
namespace worldedit {
    class CuboidRegion : public Region {
       public:
        BlockPos mainPos{-2100000000, -2100000000, -2100000000};
        BlockPos vicePos{-2100000000, -2100000000, -2100000000};
        void updateBoundingBox() override;

        explicit CuboidRegion(const BoundingBox& region, const int& dim);
        explicit CuboidRegion();

        bool expand(const std::vector<BlockPos>& changes,
                    Player* player) override;

        bool contract(const std::vector<BlockPos>& changes,
                      Player* player) override;

        bool expand(const BlockPos& change, Player* player) override;

        bool contract(const BlockPos& change, Player* player) override;

        bool shift(const BlockPos& change, Player* player) override;

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        // void forEachBlockInRegion(const std::function<void(const BlockPos &)>
        // &todo) override;

        Region* createRegion(RegionType type, BoundingBox boundingBox, int dim);
    };
}  // namespace worldedit
#endif  // WORLDEDIT_CUBOIDREGION_H