#include "command/edit/CommandHelpers.h"
#include "pattern/SingleBlockPattern.h"

namespace we {

REG_CMD(edit, hollow, "Hollow out the region") {
    struct Params {
        int              num = 0;
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
        std::shared_ptr<we::Pattern> pattern;
        if (params.block.mBlockNameHash == 0 && params.pattern.mText.empty()) {
            auto v   = SingleBlockPattern::create().value();
            v->block = DirectAccessBlocks::Air();
            pattern  = std::move(v);
        } else {
            pattern = preparePattern(ctx, params.block, params.pattern);
        }
        if (!pattern) return;

        auto& blockSource = dim->getBlockSourceFromMainChunkSource();

        try {
            ll::SmallDenseSet<BlockPos> tmp;

            for (auto const& pos : region->forEachBlockInRegion()) {
                int counts = 0;
                for (auto const& calPos : pos.getNeighbors()) {
                    if (!blockSource.getBlock(calPos).isAir()) {
                        counts++;
                    }
                }
                if (counts == 6) {
                    tmp.insert(pos);
                }
            }

            for (int manhattan = 0; manhattan < params.num; ++manhattan) {
                ll::SmallDenseSet<BlockPos> tmp2;
                for (auto const& pos : tmp) {
                    if (countNeighbors6InSet(pos, tmp) == 6) {
                        tmp2.insert(pos);
                    }
                }
                tmp = std::move(tmp2);
            }

            auto record = std::make_shared<HistoryRecord>();
            for (auto const& pos : region->forEachBlockInRegion()) {
                if (tmp.contains(pos)) {
                    record->record(*lctx, blockSource, {pos, pattern->pickBlock(pos)});
                }
            }

            auto changed = record->apply(*lctx, blockSource);
            lctx->history.addRecord(std::move(record));
            if (changed == 0) {
                ctx.success("No blocks were changed");
            } else {
                ctx.success("Hollowed {0} block(s)", changed);
            }
        } catch (std::bad_alloc const&) {
            ctx.error("Out of memory");
        }
    };

    command.overload<Params>().optional("num").optional("block").execute(
        CmdCtxBuilder{} | executor
    );
    command.overload<Params>().optional("num").optional("pattern").execute(
        CmdCtxBuilder{} | executor
    );
}
} // namespace we
