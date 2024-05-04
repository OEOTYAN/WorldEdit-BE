#include "data/PlayerStateManager.h"
#include "region/LoftRegion.h"
#include "worldedit/WorldEdit.h"

#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>

#include <mc/world/level/dimension/Dimension.h>

namespace we {

struct Params {
    ll::command::Optional<CommandPositionFloat> pos;
};

void setupPos1() {
    auto& config = WorldEdit::getInstance().getConfig().commands.region.pos1;
    if (!config.enabled) {
        return;
    }
    ll::command::CommandRegistrar::getInstance()
        .getOrCreateCommand(
            "pos1",
            "sets the main position to the given or current position"_tr(),
            config.permission
        )
        .overload<Params>()
        .optional("pos")
        .execute([](CommandOrigin const& origin,
                    CommandOutput&       output,
                    Params const&        params,
                    ::Command const&     cmd) {
            DimensionType dimId;
            if (auto* dim = origin.getDimension(); dim) {
                dimId = dim->getDimensionId();
            } else {
                output.error("origin doesn't have dimension"_tr());
                return;
            }
            auto&    manager = WorldEdit::getInstance().getPlayerStateManager();
            auto     state   = manager.getOrCreate(origin);
            BlockPos pos     = params.pos
                               .transform([&](auto& cmdPos) {
                                   return origin.getExecutePosition(cmd.mVersion, cmdPos);
                               })
                               .value_or(origin.getWorldPosition());
            if (state->setMainPos({pos, dimId})) {
                output.success("set main position at {0}"_tr(pos));
            } else {
                output.error("can't set main position at {0}"_tr(pos));
            }
        });
}

void setupPos2() {
    auto& config = WorldEdit::getInstance().getConfig().commands.region.pos2;
    if (!config.enabled) {
        return;
    }
    ll::command::CommandRegistrar::getInstance()
        .getOrCreateCommand(
            "pos2",
            "sets the off position to the given or current position"_tr(),
            config.permission
        )
        .overload<Params>()
        .optional("pos")
        .execute([](CommandOrigin const& origin,
                    CommandOutput&       output,
                    Params const&        params,
                    ::Command const&     cmd) {
            DimensionType dimId;
            if (auto* dim = origin.getDimension(); dim) {
                dimId = dim->getDimensionId();
            } else {
                output.error("origin doesn't have dimension"_tr());
                return;
            }
            auto&    manager = WorldEdit::getInstance().getPlayerStateManager();
            auto     state   = manager.getOrCreate(origin);
            BlockPos pos     = params.pos
                               .transform([&](auto& cmdPos) {
                                   return origin.getExecutePosition(cmd.mVersion, cmdPos);
                               })
                               .value_or(origin.getWorldPosition());
            if (state->setOffPos({pos, dimId})) {
                output.success("set off position at {0}"_tr(pos));
            } else {
                output.error("can't set off position at {0}"_tr(pos));
            }
        });
}

void setupHpos1() {
    auto& config = WorldEdit::getInstance().getConfig().commands.region.hpos1;
    if (!config.enabled) {
        return;
    }
    ll::command::CommandRegistrar::getInstance()
        .getOrCreateCommand(
            "hpos1",
            "sets the main position to the cursor position"_tr(),
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
            auto& manager   = WorldEdit::getInstance().getPlayerStateManager();
            auto  state     = manager.getOrCreate(origin);
            auto  hitResult = static_cast<Player*>(e)->traceRay(
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
            if (state->setMainPos({hitResult.mBlockPos, dimId})) {
                output.success("set main position at {0}"_tr(hitResult.mBlockPos));
            } else {
                output.error("can't set main position at {0}"_tr(hitResult.mBlockPos));
            }
        });
}

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
            auto& manager   = WorldEdit::getInstance().getPlayerStateManager();
            auto  state     = manager.getOrCreate(origin);
            auto  hitResult = static_cast<Player*>(e)->traceRay(
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

void setupPos() {
    setupPos1();
    setupPos2();
    setupHpos1();
    setupHpos2();
}
} // namespace we
