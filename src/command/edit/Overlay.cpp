#include "command/edit/CommandHelpers.h"

namespace we {
REG_CMD(edit, overlay, "Set the top blocks of the region") {
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
        for (auto pos : forEachTopBlock(*region, blockSource)) {
            pos.y++;
            if (region->contains(pos))
                record->record(*lctx, blockSource, {pos, pattern->pickBlock(pos)});
        }

        auto changed = record->apply(*lctx, blockSource);
        lctx->history.addRecord(std::move(record));
        if (changed == 0) {
            ctx.success("No blocks were changed");
        } else {
            ctx.success("Overlayed {0} block(s)", changed);
        }
    };

    command.overload<Params>().required("block").execute(CmdCtxBuilder{} | executor);
    command.overload<Params>().required("pattern").execute(CmdCtxBuilder{} | executor);
}
} // namespace we