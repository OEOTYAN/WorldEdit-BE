#include "command/CommandMacro.h"

namespace we {
REG_CMD(edit, set, "Set blocks in the region to a specific block type") {
    struct Params {
        CommandBlockName                        block;
        std::vector<BlockStateCommandParam>     states;
        ll::command::Optional<CommandBlockName> exblock;
        std::vector<BlockStateCommandParam>     exstates;
    };
    command.overload<Params>()
        .required("block")
        .optional("states")
        .optional("exblock")
        .optional("exstates")
        .execute(
            CmdCtxBuilder{} | [](CommandContextRef const& ctx, Params const& params) {
                auto lctx = checkLocalContext(ctx);
                if (!lctx) return;
                auto region = checkRegion(ctx);
                if (!region) return;
                auto dim = checkDimension(ctx);
                if (!dim) return;
                auto& blockSource = dim->getBlockSourceFromMainChunkSource();

                auto getBlock = [&](CommandBlockName const&                    b,
                                    std::vector<BlockStateCommandParam> const& p) {
                    optional_ref<Block const> block;
                    auto&                     name = b.getBlockName();
                    auto states = BlockStateCommandParam::toStateMap(p);
                    if (!states) {
                        ctx.output.error("Invalid block states:");
                        states.error().log(ctx.output);
                        return block;
                    }
                    block = Block::tryGetFromRegistry(name, states.value());
                    if (!block) {
                        ctx.output.error("Unknown block type: {0}", name);
                    }
                    return block;
                };

                auto block = getBlock(params.block, params.states);
                if (!block) return;
                optional_ref<Block const> exblock;
                if (params.exblock) {
                    exblock = getBlock(params.exblock, params.exstates);
                    if (!exblock) return;
                }
                OperationData::Operation op;
                auto& blockOp = op.operation.emplace<OperationData::BlockOperation>();
                blockOp.block = block;
                if (exblock) blockOp.extraBlock = exblock;

                auto record = std::make_shared<HistoryRecord>();

                region->forEachBlockInRegion([&](BlockPos const& pos) {
                    op.pos = pos;
                    record->record(*lctx, blockSource, op);
                });
                auto num = record->apply(*lctx, blockSource);
                if (!lctx->history.addRecord(std::move(record))) {
                    ctx.output.error("Failed to set");
                    return;
                }
                if (params.exblock) {
                    ctx.output.success(
                        "Set {0} blocks to {1} with {2}",
                        num,
                        exblock->getTypeName(),
                        block->getTypeName()
                    );
                } else {
                    ctx.output
                        .success("Set {0} blocks to {1}", num, block->getTypeName());
                }
            }
        );
}
} // namespace we
