#include "command/CommandMacro.h"

namespace we {
REG_CMD(region, hpos2, "sets the off position to the cursor position") {
    command.overload().execute(CmdCtxBuilder{} | [](CommandContextRef const& ctx) {
        auto player = checkPlayer(ctx);
        if (!player) return;
        auto pctx      = getPlayerContext(ctx);
        auto hitResult = player->traceRay(
            WorldEdit::getInstance().getConfig().player_state.maximum_trace_length,
            false
        );
        if (!hitResult) {
            ctx.error("can't trace cursor position");
            return;
        }
        if (hitResult.mIsHitLiquid && !player->isImmersedInWater()) {
            hitResult.mBlockPos = hitResult.mLiquid;
        }
        if (pctx->setOffPos({hitResult.mBlockPos, player->getDimensionId()})) {
            ctx.success("set off position at {0}", hitResult.mBlockPos);
        } else {
            ctx.error("can't set off position at {0}", hitResult.mBlockPos);
        }
    });
};
} // namespace we