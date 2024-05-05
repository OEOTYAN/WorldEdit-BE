#include "data/PlayerStateManager.h"
#include "region/LoftRegion.h"
#include "worldedit/WorldEdit.h"

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
            auto state =
                WorldEdit::getInstance().getPlayerStateManager().getOrCreate(origin);
            BlockPos pos = params.pos
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
} // namespace we
