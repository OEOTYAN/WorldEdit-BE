#include "command/Commands.h"
#include "data/PlayerStateManager.h"
#include "region/LoftRegion.h"
#include "worldedit/WorldEdit.h"

namespace we {

struct Params {
    bool cycle;
};

static bool _ = addSetup("loftcycle", [] {
    auto& config = WorldEdit::getInstance().getConfig().commands.region.loftcycle;
    if (!config.enabled) {
        return;
    }
    auto& command = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "loftcycle",
        "set loft region curve is connected end to end"_tr(),
        config.permission
    );

    command.overload<Params>().required("cycle").execute(
        [](CommandOrigin const& origin, CommandOutput& output, Params const& params) {
            auto state = WorldEdit::getInstance().getPlayerStateManager().get(origin);
            if (!state) {
                output.error("origin didn't have state"_tr());
                return;
            }
            if (!state->region || state->region->getType() != RegionType::Loft) {
                output.error("origin didn't selected loft region"_tr());
                return;
            }
            std::static_pointer_cast<LoftRegion>(state->region)->setCycle(params.cycle);
            output.success("set loft region cycle to {0}"_tr(params.cycle));
        }
    );
});
} // namespace we
