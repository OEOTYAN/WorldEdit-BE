#include "command/CommandMacro.h"

namespace we {
REG_CMD(region, hpos1, "sets the main position to the cursor position") {
    command.overload().execute(CmdCtxBuilder{} | [](CommandContextRef const& ctx) {
        auto player = checkPlayer(ctx);
        if (!player) return;
        auto lctx      = getLocalContext(ctx);
        auto hitResult = player->traceRay(
            WorldEdit::getInstance().getConfig().player_state.maximum_trace_length,
            false
        );
        if (!hitResult) {
            ctx.error("can't trace cursor position");
            return;
        }
        if (hitResult.mIsHitLiquid && !player->_isHeadInWater()) {
            hitResult.mBlock = hitResult.mLiquid;
        }
        if (lctx->setMainPos({hitResult.mBlock, player->getDimensionId()})) {
            ctx.success("set main position at {0}", hitResult.mBlock);
        } else {
            ctx.error("can't set main position at {0}", hitResult.mBlock);
        }
    });
}
} // namespace we
