#include "command/CommandMacro.h"

namespace we {
static void executeRedo(CommandContextRef const& ctx, int steps) {
    auto lctx = getLocalContext(ctx);
    auto dim  = checkDimension(ctx);
    if (!dim) return;
    auto& blockSource = dim->getBlockSourceFromMainChunkSource();

    ll::DenseSet<BlockPos> redonePositions;
    size_t                 redoneSteps  = 0;
    size_t                 redoneBlocks = 0;

    std::vector<std::shared_ptr<HistoryRecord>> records;

    if (steps < 0) {
        while (auto record = lctx->history.redo()) {
            records.push_back(std::move(record));
        }
    } else {
        for (int i = 0; i < steps; ++i) {
            auto record = lctx->history.redo();
            if (!record) {
                break;
            }
            records.push_back(std::move(record));
        }
    }

    for (auto it = records.rbegin(); it != records.rend(); ++it) {
        auto& record  = *it;
        redoneBlocks += record->redo(*lctx, blockSource, [&](BlockPos const& pos) {
            return redonePositions.emplace(pos).second;
        });
        redoneSteps++;
    }
    ctx.success("Redone {0} step(s), {1} block(s) modified", redoneSteps, redoneBlocks);
}

REG_CMD(history, redo, "Redo the last action") {
    struct Params {
        int steps = 1;
    };
    command.overload<Params>().optional("steps").execute(
        CmdCtxBuilder{} | [](CommandContextRef const& ctx, Params const& params) {
            executeRedo(ctx, params.steps);
        }
    );
    command.overload().text("all").execute(
        CmdCtxBuilder{} | [](CommandContextRef const& ctx) { executeRedo(ctx, -1); }
    );
}
} // namespace we
