#include "command/CommandMacro.h"

namespace we {
REG_CMD(region, hpos1, "sets the main position to the cursor position") {
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
        if (pctx->setMainPos({hitResult.mBlockPos, player->getDimensionId()})) {
            ctx.success("set main position at {0}", hitResult.mBlockPos);
        } else {
            ctx.error("can't set main position at {0}", hitResult.mBlockPos);
        }
    });
};
} // namespace we
