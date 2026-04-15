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

    bool empty() const {
        return !block && !extraBlock && !blockActor;
    }

    bool apply(LocalContext& context, BlockSource& source, BlockPos const& pos) const;
    BlockOperation record(
        LocalContext&   context,
        BlockSource&    source,
        BlockPos const& pos,
        bool&           deferred
    ) const;
};
struct BiomeOperation {
    optional_ref<Biome const> biome = nullptr;

    bool empty() const { return !biome; }

    bool apply(LocalContext& context, BlockSource& source, BlockPos const& pos) const;
    BiomeOperation record(
        LocalContext&   context,
        BlockSource&    source,
        BlockPos const& pos,
        bool&           deferred
    ) const;
};
struct EntityOperation {
    bool empty() const { return true; }

    bool apply(LocalContext&, BlockSource&, BlockPos const&) const { return false; }
    EntityOperation record(LocalContext&, BlockSource&, BlockPos const&, bool&) const {
        return EntityOperation{};
    }
};
class Operation {
public:
    using VariantType = std::variant<BlockOperation, BiomeOperation, EntityOperation>;
    BlockPos    pos;
    VariantType operation;

    Operation() = default;

    template <typename Op>
    Operation(BlockPos const& pos, Op&& op) : pos(pos), operation(std::forward<Op>(op)) {}

    bool empty() const {
        return std::visit(
            [&](auto&& arg) -> bool { return arg.empty(); },
            operation
        );
    }

    bool apply(LocalContext& context, BlockSource& source) const {
        return std::visit(
            [&](auto&& arg) -> bool { return arg.apply(context, source, pos); },
            operation
        );
    }
    Operation record(LocalContext& context, BlockSource& source, bool& deferred) const {
        return Operation{
            pos,
            std::visit(
                [&](auto&& arg) -> VariantType {
                    return arg.record(context, source, pos, deferred);
                },
                operation
            )
        };
    }
};

} // namespace we
