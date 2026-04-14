#include "command/CommandMacro.h"

namespace we {
static void executeUndo(CommandContextRef const& ctx, int steps) {
    auto lctx = getLocalContext(ctx);
    auto dim  = checkDimension(ctx);
    if (!dim) return;
    auto& blockSource = dim->getBlockSourceFromMainChunkSource();

    ll::DenseSet<BlockPos> undonePositions;
    size_t                 undoneSteps  = 0;
    size_t                 undoneBlocks = 0;

    std::vector<std::shared_ptr<HistoryRecord>> records;

    if (steps < 0) {
        while (auto record = lctx->history.undo()) {
            records.push_back(std::move(record));
        }
    } else {
        for (int i = 0; i < steps; ++i) {
            auto record = lctx->history.undo();
            if (!record) {
                break;
            }
            records.push_back(std::move(record));
        }
    }

    for (auto it = records.rbegin(); it != records.rend(); ++it) {
        auto& record  = *it;
        undoneBlocks += record->undo(*lctx, blockSource, [&](BlockPos const& pos) {
            return undonePositions.emplace(pos).second;
        });
        undoneSteps++;
    }
    ctx.success("Undone {0} step(s), {1} block(s) modified", undoneSteps, undoneBlocks);
}

REG_CMD(history, undo, "Undo the last action") {
    struct Params {
        int steps = 1;
    };
    command.overload<Params>().optional("steps").execute(
        CmdCtxBuilder{} | [](CommandContextRef const& ctx, Params const& params) {
            executeUndo(ctx, params.steps);
        }
    );
    command.overload().text("all").execute(
        CmdCtxBuilder{} | [](CommandContextRef const& ctx) { executeUndo(ctx, -1); }
    );
}
} // namespace we
