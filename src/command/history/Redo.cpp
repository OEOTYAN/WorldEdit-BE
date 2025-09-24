#include "command/CommandMacro.h"

namespace we {
REG_CMD(history, redo, "Redo the last action") {
    struct Params {
        int steps = 1;
    };
    command.overload<Params>().optional("steps").execute(
        CmdCtxBuilder{} | [](CommandContextRef const& ctx, Params const& params) {
            auto lctx = getLocalContext(ctx);
            auto dim  = checkDimension(ctx);
            if (!dim) return;
            auto& blockSource = dim->getBlockSourceFromMainChunkSource();

            ll::DenseSet<BlockPos> redonePositions;
            size_t                 redoneSteps  = 0;
            size_t                 redoneBlocks = 0;

            std::vector<std::shared_ptr<HistoryRecord>> records;

            for (int i = 0; i < params.steps; ++i) {
                auto record = lctx->history.redo();
                if (!record) {
                    break;
                }
                records.push_back(record);
            }
            for (auto it = records.rbegin(); it != records.rend(); ++it) {
                auto& record = *it;
                redoneBlocks +=
                    record->redo(*lctx, blockSource, [&](BlockPos const& pos) {
                        return redonePositions.emplace(pos).second;
                    });
                redoneSteps++;
            }
            ctx.success(
                "Redone {} step(s), {} block(s) modified",
                redoneSteps,
                redoneBlocks
            );
        }
    );
}
} // namespace we
