#include "command/edit/CommandHelpers.h"
#include "utils/Math.h"

namespace we {

REG_CMD(edit, line, "Draw lines between region vertices") {
    struct Params {
        CommandBlockName block{};
        CommandRawText   pattern;
        float            radius = 0.0f;
        struct VaArgs {
            bool hollow = false;
        } args;
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

        auto type = region->getType();
        if (type != RegionType::Cuboid && type != RegionType::Convex
            && type != RegionType::Loft) {
            ctx.error("Line only supports cuboid, convex and loft regions");
            return;
        }

        auto& blockSource = dim->getBlockSourceFromMainChunkSource();

        int radius = static_cast<int>(params.radius);

        ll::SmallDenseSet<BlockPos> tmp;

        region->forEachLine([&](BlockPos const& pos1, BlockPos const& pos2) {
            plotLine(pos1, pos2, [&](BlockPos const& pos) {
                if (params.radius == 0) {
                    tmp.insert(pos);
                } else {
                    auto box = BoundingBox(pos - (radius + 1), pos + (radius + 1));
                    for (auto const& posk : box.forEachPos()) {
                        if ((pos - posk).length() <= 0.5f + params.radius) {
                            tmp.insert(posk);
                        }
                    }
                }
            });
        });

        if (params.args.hollow) {
            ll::SmallDenseSet<BlockPos> tmp2;
            for (auto const& pos : tmp) {
                if (countNeighbors6InSet(pos, tmp) < 6) {
                    tmp2.insert(pos);
                }
            }
            tmp = std::move(tmp2);
        }

        auto record = std::make_shared<HistoryRecord>();
        for (auto const& pos : tmp) {
            record->record(*lctx, blockSource, {pos, pattern->pickBlock(pos)});
        }

        auto changed = record->apply(*lctx, blockSource);
        lctx->history.addRecord(std::move(record));
        if (changed == 0) {
            ctx.success("No blocks were changed");
        } else {
            ctx.success("Drew {0} line block(s)", changed);
        }
    };

    command.overload<Params>()
        .required("block")
        .optional("radius")
        .optional("args")
        .execute(CmdCtxBuilder{} | executor);
    command.overload<Params>()
        .required("pattern")
        .optional("radius")
        .optional("args")
        .execute(CmdCtxBuilder{} | executor);
}
} // namespace we
