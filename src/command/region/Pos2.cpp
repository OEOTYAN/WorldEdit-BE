#include "command/Commands.h"
#include "data/PlayerStateManager.h"
#include "region/LoftRegion.h"
#include "worldedit/WorldEdit.h"

namespace we {
struct Params {
    ll::command::Optional<CommandPositionFloat> pos;
};
static bool _ = addSetup("pos2", [] {
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
            auto state =
                WorldEdit::getInstance().getPlayerStateManager().getOrCreate(origin);
            BlockPos pos = params.pos
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
});
} // namespace we
