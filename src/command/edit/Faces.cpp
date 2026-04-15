#include "command/edit/CommandHelpers.h"

namespace we {
REG_CMD(edit, faces, "Fill the outer faces of the region") {
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
        region->forEachBlockInRegion([&](BlockPos const& pos) {
            int neighborCount = 0;
            for (auto const& neighbor : pos.getNeighbors()) {
                neighborCount += region->contains(neighbor) ? 1 : 0;
            }
            if (neighborCount >= 6) {
                return;
            }
            record->record(*lctx, blockSource, {pos, pattern->pickBlock(pos)});
        });

        auto changed = record->apply(*lctx, blockSource);
        lctx->history.addRecord(std::move(record));
        if (changed == 0) {
            ctx.success("No blocks were changed");
        } else {
            ctx.success("Set {0} face block(s)", changed);
        }
    };

    command.overload<Params>().required("block").execute(CmdCtxBuilder{} | executor);
    command.overload<Params>().required("pattern").execute(CmdCtxBuilder{} | executor);
}
} // namespace we