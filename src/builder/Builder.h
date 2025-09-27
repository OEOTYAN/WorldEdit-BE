#pragma once

#include "worldedit/Global.h"

namespace we {

class LocalContext;

enum class BuilderType {
    None,
    Inplace,
    InplaceNoNc,
    Bot,
};

class Builder {
    LocalContext& context;
    BuilderType type;
public:
    Builder(LocalContext& context) : context(context){}

    BuilderType getType() const { return type; }

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
    static std::unique_ptr<Builder> create(BuilderType type, LocalContext& context);
};

} // namespace we