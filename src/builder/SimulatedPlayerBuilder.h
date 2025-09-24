#pragma once

#include "builder/Builder.h"

namespace we {
class LocalContext;
class SimulatedPlayerBuilder : public Builder {
public:
    SimulatedPlayerBuilder(LocalContext& context) : Builder(context) {}

    virtual ~SimulatedPlayerBuilder() = default;

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