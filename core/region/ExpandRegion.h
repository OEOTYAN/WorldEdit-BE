//
// Created by OEOTYAN on 2021/2/8.
//

#ifndef WORLDEDIT_EXPANDREGION_H
#define WORLDEDIT_EXPANDREGION_H

#include "Region.h"
namespace worldedit {
    class ExpandRegion : public Region {
       public:
        BlockPos mainPos{-2100000000, -2100000000, -2100000000};
        BlockPos vicePos{-2100000000, -2100000000, -2100000000};
        void updateBoundingBox() override;

        explicit ExpandRegion(const BoundingBox& region, const int& dim);

        bool expand(const std::vector<BlockPos>& changes,
                    Player* player) override;

        bool contract(const std::vector<BlockPos>& changes,
                      Player* player) override;

        bool expand(const BlockPos& change, Player* player) override;

        bool contract(const BlockPos& change, Player* player) override;

        bool shift(const BlockPos& change, Player* player) override;

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;
    };
}  // namespace worldedit
#endif  // WORLDEDIT_EXPANDREGION_H