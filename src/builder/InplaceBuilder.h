#pragma once

#include "builder/Builder.h"

namespace we {
class LocalContext;
class InplaceBuilder : public Builder {

    void fireBlockChanged(
        BlockSource&     source,
        BlockPos const&  pos,
        BlockLayer::Type layer,
        Block const&     oldBlock,
        Block const&     newBlock
    ) const;

public:
    BlockUpdateFlag::Type updateFlags;

    InplaceBuilder(
        LocalContext&         context,
        BlockUpdateFlag::Type updateFlags = BlockUpdateFlag::All
    )
    : Builder(context),
      updateFlags(updateFlags) {}

    virtual ~InplaceBuilder() = default;

    virtual bool setBlock(
        BlockSource&,
        BlockPos const&,
        Block const&,
        std::shared_ptr<BlockActor>
    ) const;
    virtual bool setExtraBlock(BlockSource&, BlockPos const&, Block const&) const;
    virtual bool setBiome(BlockSource&, BlockPos const&, Biome const&) const;
};

} // namespace we