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

class Builder : public std::enable_shared_from_this<Builder> {
    BuilderType   type;
    
protected:
    LocalContext& context;

public:
    Builder(LocalContext& context) : context(context) {}

    BuilderType getType() const { return type; }

    virtual ~Builder() = default;

    virtual void setup() {}

    virtual void remove() {}

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
    static std::shared_ptr<Builder> create(BuilderType type, LocalContext& context);
};

} // namespace we