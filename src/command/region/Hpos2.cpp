#include "data/PlayerStateManager.h"
#include "region/LoftRegion.h"
#include "worldedit/WorldEdit.h"

namespace we {
void setupHpos2() {
    auto& config = WorldEdit::getInstance().getConfig().commands.region.hpos2;
    if (!config.enabled) {
        return;
    }
    ll::command::CommandRegistrar::getInstance()
        .getOrCreateCommand(
            "hpos2",
            "sets the off position to the cursor position"_tr(),
            config.permission
        )
        .overload()
        .execute([](CommandOrigin const& origin, CommandOutput& output) {
            DimensionType dimId;
            auto*         e = origin.getEntity();
            if (e && e->isPlayer()) {
                dimId = e->getDimensionId();
            } else {
                output.error("origin isn't player"_tr());
                return;
            }
            auto state =
                WorldEdit::getInstance().getPlayerStateManager().getOrCreate(origin);
            auto hitResult = static_cast<Player*>(e)->traceRay(
                WorldEdit::getInstance().getConfig().player_state.maximum_trace_length,
                false
            );
            if (!hitResult) {
                output.error("can't trace cursor position"_tr());
                return;
            }
            if (hitResult.mIsHitLiquid && !static_cast<Player*>(e)->isImmersedInWater()) {
                hitResult.mBlockPos = hitResult.mLiquid;
                hitResult.mFacing   = hitResult.mLiquidFacing;
            }
            if (state->setOffPos({hitResult.mBlockPos, dimId})) {
                output.success("set off position at {0}"_tr(hitResult.mBlockPos));
            } else {
                output.error("can't set off position at {0}"_tr(hitResult.mBlockPos));
            }
        });
}
} // namespace we
