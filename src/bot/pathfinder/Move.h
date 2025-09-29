#pragma once

#include "worldedit/Global.h"
#include <string>
#include <vector>

namespace we::bot {

struct BlockToBreak {
    BlockPos     pos;
    Block const* block;

    BlockToBreak(BlockPos const& p, Block const* b) : pos(p), block(b) {}
};

struct BlockToPlace {
    BlockPos     pos;
    Block const* block;

    BlockToPlace(BlockPos const& p, Block const* b) : pos(p), block(b) {}
};

class Move : public BlockPos {
public:
    int                       remainingBlocks;
    double                    cost;
    std::vector<BlockToBreak> toBreak;
    std::vector<BlockToPlace> toPlace;
    bool                      parkour;
    uint64_t                  hash;

    Move(
        int                       x,
        int                       y,
        int                       z,
        int                       remainingBlocks,
        double                    cost,
        std::vector<BlockToBreak> toBreak = {},
        std::vector<BlockToPlace> toPlace = {},
        bool                      parkour = false
    );

    Move(
        BlockPos const&           pos,
        int                       remainingBlocks,
        double                    cost,
        std::vector<BlockToBreak> toBreak = {},
        std::vector<BlockToPlace> toPlace = {},
        bool                      parkour = false
    );

    BlockPos getBlockPos() const { return {x, y, z}; }
    Vec3     getVec3() const { return getBlockPos().bottomCenter(); }

    double distanceSquared(Move const& other) const;
    double distance(Move const& other) const;

    Move min(Move const& other) const;
    Move max(Move const& other) const;

    bool operator==(Move const& other) const;
};

} // namespace we::bot