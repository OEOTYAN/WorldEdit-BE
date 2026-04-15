#include "command/edit/CommandHelpers.h"

namespace we {
REG_CMD(edit, set, "Set blocks in the region to a specific block type") {
    struct Params {
        CommandBlockName block{};
        CommandRawText   pattern;
    };

    auto executor = [](CommandContextRef const& ctx, Params const& params) {
        auto lctx = checkLocalContext(ctx);
        if (!lctx) return;
        auto region = checkRegion(ctx);
        if (!region) return;
        auto dim = checkDimension(ctx);
        if (!dim) return;
        auto pattern = preparePattern(ctx, params.block, params.pattern);
        if (!pattern) return;

        auto& blockSource = dim->getBlockSourceFromMainChunkSource();

        auto record = std::make_shared<HistoryRecord>();
        for (auto const& pos : region->forEachBlockInRegion()) {
            record->record(*lctx, blockSource, {pos, pattern->pickBlock(pos)});
        }
        auto num = record->apply(*lctx, blockSource);
        lctx->history.addRecord(std::move(record));
        if (num == 0) {
            ctx.success("No blocks were changed");
        } else {
            ctx.success("Set {0} blocks", num);
        }
    };

    command.overload<Params>().required("block").execute(CmdCtxBuilder{} | executor);
    command.overload<Params>().required("pattern").execute(CmdCtxBuilder{} | executor);
}
} // namespace we
