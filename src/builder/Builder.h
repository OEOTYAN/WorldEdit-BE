#pragma once

#include "worldedit/Global.h"

namespace we {
class LocalContext;
class Builder {
    LocalContext& context;
public:
    Builder(LocalContext& context) : context(context) {}

    virtual ~Builder() = default;

    virtual bool setBlock(
        BlockSource&,
        BlockPos const&,
        Block const&,
        std::shared_ptr<BlockActor>
    ) const {
        return false;
    }
    virtual bool setExtraBlock(BlockSource&, BlockPos const&, Block const&) const {
        return false;
    }
    virtual bool setBiome(BlockSource&, BlockPos const&, Biome const&) const {
        return false;
    }
};

} // namespace we