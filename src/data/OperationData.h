#pragma once

#include "worldedit/Global.h"
#include <mc/world/actor/Actor.h>
#include <mc/world/level/biome/Biome.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/actor/BlockActor.h>

namespace we {
class LocalContext;
class OperationData {
public:
    struct BlockOperation {
        optional_ref<Block const>   block      = nullptr;
        optional_ref<Block const>   extraBlock = nullptr;
        std::shared_ptr<BlockActor> blockActor = nullptr;
        optional_ref<Biome const>   biome      = nullptr;
        bool apply(LocalContext& context, BlockSource& region, BlockPos const& pos) const;
        BlockOperation
        record(LocalContext& context, BlockSource& region, BlockPos const& pos) const;
    };
    struct EntityOperation {
        bool
        apply(LocalContext& context, BlockSource& region, BlockPos const& pos) const {
            return false;
        }
        EntityOperation
        record(LocalContext& context, BlockSource& region, BlockPos const& pos) const {
            return EntityOperation{};
        }
    };
    class Operation {
    public:
        using VariantType = std::variant<BlockOperation, EntityOperation>;
        BlockPos    pos;
        VariantType operation;

        bool apply(LocalContext& context, BlockSource& region) const {
            return std::visit(
                [&](auto&& arg) -> bool { return arg.apply(context, region, pos); },
                operation
            );
        }
        Operation record(LocalContext& context, BlockSource& region) const {
            return Operation{
                pos,
                std::visit(
                    [&](auto&& arg) -> VariantType {
                        return arg.record(context, region, pos);
                    },
                    operation
                )
            };
        }
    };


    std::vector<Operation> operations;


    size_t getEstimatedSize() const noexcept {
        size_t totalSize  = sizeof(*this);
        totalSize        += operations.size() * sizeof(Operation);
        return totalSize;
    }
};

} // namespace we