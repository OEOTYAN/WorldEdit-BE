#include "command/CommandMacro.h"

namespace we {
REG_CMD(history, undo, "Undo the last action") {
    struct Params {
        int steps = 1;
    };
    command.overload<Params>().optional("steps").execute(
        CmdCtxBuilder{} | [](CommandContextRef const& ctx, Params const& params) {
            auto lctx = getLocalContext(ctx);
            auto dim  = checkDimension(ctx);
            if (!dim) return;
            auto& blockSource = dim->getBlockSourceFromMainChunkSource();

            ll::DenseSet<BlockPos> undonePositions;
            size_t                 undoneSteps  = 0;
            size_t                 undoneBlocks = 0;

            std::vector<std::shared_ptr<HistoryRecord>> records;

            for (int i = 0; i < params.steps; ++i) {
                auto record = lctx->history.undo();
                if (!record) {
                    break;
                }
                records.push_back(record);
            }
            for (auto it = records.rbegin(); it != records.rend(); ++it) {
                auto& record = *it;
                undoneBlocks +=
                    record->undo(*lctx, blockSource, [&](BlockPos const& pos) {
                        return undonePositions.emplace(pos).second;
                    });
                undoneSteps++;
            }
            ctx.success(
                "Undone {} step(s), {} block(s) modified",
                undoneSteps,
                undoneBlocks
            );
        }
    );
}
} // namespace we
