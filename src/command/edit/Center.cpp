#include "command/edit/CommandHelpers.h"

namespace we {
REG_CMD(edit, center, "Set the center blocks of the region") {
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

        auto        center = region->getCenter();
        BoundingBox centerBox;
        centerBox.min.x = static_cast<int>(std::floor(center.x - 0.49f));
        centerBox.min.y = static_cast<int>(std::floor(center.y - 0.49f));
        centerBox.min.z = static_cast<int>(std::floor(center.z - 0.49f));
        centerBox.max.x = static_cast<int>(std::floor(center.x + 0.49f));
        centerBox.max.y = static_cast<int>(std::floor(center.y + 0.49f));
        centerBox.max.z = static_cast<int>(std::floor(center.z + 0.49f));

        auto record = std::make_shared<HistoryRecord>();
        for (auto const& pos : centerBox.forEachPos()) {
            record->record(*lctx, blockSource, {pos, pattern->pickBlock(pos)});
        }

        auto changed = record->apply(*lctx, blockSource);
        lctx->history.addRecord(std::move(record));
        if (changed == 0) {
            ctx.success("No blocks were changed");
        } else {
            ctx.success("Set {0} center block(s)", changed);
        }
    };

    command.overload<Params>().required("block").execute(CmdCtxBuilder{} | executor);
    command.overload<Params>().required("pattern").execute(CmdCtxBuilder{} | executor);
}
} // namespace we