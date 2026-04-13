#pragma once

#include "worldedit/Global.h"
#include <mc/world/actor/Actor.h>
#include <mc/world/level/biome/Biome.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/actor/BlockActor.h>

namespace we {
class LocalContext;
struct BlockOperation {
    optional_ref<Block const>   block      = nullptr;
    optional_ref<Block const>   extraBlock = nullptr;
    std::shared_ptr<BlockActor> blockActor = nullptr;
    optional_ref<Biome const>   biome      = nullptr;

    bool empty() const {
        return !block && !extraBlock && !blockActor && !biome;
    }

    bool apply(LocalContext& context, BlockSource& source, BlockPos const& pos) const;
    BlockOperation record(
        LocalContext&   context,
        BlockSource&    source,
        BlockPos const& pos,
        bool&           valid
    ) const;
};
struct EntityOperation {
    bool apply(LocalContext&, BlockSource&, BlockPos const&) const { return false; }
    EntityOperation record(LocalContext&, BlockSource&, BlockPos const&, bool&) const {
        return EntityOperation{};
    }
};
class Operation {
public:
    using VariantType = std::variant<BlockOperation, EntityOperation>;
    BlockPos    pos;
    VariantType operation;

    bool apply(LocalContext& context, BlockSource& source) const {
        return std::visit(
            [&](auto&& arg) -> bool { return arg.apply(context, source, pos); },
            operation
        );
    }
    Operation record(LocalContext& context, BlockSource& source, bool& valid) const {
        return Operation{
            pos,
            std::visit(
                [&](auto&& arg) -> VariantType {
                    return arg.record(context, source, pos, valid);
                },
                operation
            )
        };
    }
};

} // namespace we