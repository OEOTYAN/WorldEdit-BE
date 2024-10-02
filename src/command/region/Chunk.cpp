#include "command/Commands.h"
#include "data/PlayerStateManager.h"
#include "region/LoftRegion.h"
#include "worldedit/WorldEdit.h"

namespace we {
static bool _ = addSetup("chunk", [] {
    auto& config = WorldEdit::getInstance().getConfig().commands.region.chunk;
    if (!config.enabled) {
        return;
    }
    ll::command::CommandRegistrar::getInstance()
        .getOrCreateCommand("chunk", "select current chunk"_tr(), config.permission)
        .overload()
        .execute([](CommandOrigin const& origin, CommandOutput& output) {
            Dimension* dim;
            if (dim = origin.getDimension(); !dim) {
                output.error("origin doesn't have dimension"_tr());
                return;
            }
            auto state =
                WorldEdit::getInstance().getPlayerStateManager().getOrCreate(origin);

            auto& range = dim->getHeightRange();
            auto  pos   = origin.getBlockPosition();
            pos.x       = (pos.x >> 4) << 4;
            pos.z       = (pos.z >> 4) << 4;
            pos.y       = range.min;

            state->region = Region::create(
                RegionType::Cuboid,
                dim->getDimensionId(),
                {
                    pos,
                    {pos.x + 15, range.max - 1, pos.z + 15}
            }
            );
            state->regionType = RegionType::Cuboid;
            output.success("chunk selected"_tr());
        });
});
} // namespace we
