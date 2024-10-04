#include "command/CommandMacro.h"

namespace we {
REG_CMD(region, chunk, "select current chunk") {
    command.overload().execute(CmdCtxBuilder{} | [](CommandContextRef const& ctx) {
        auto dim = checkDimension(ctx);
        if (!dim) return;
        auto  pctx  = getPlayerContext(ctx);
        auto& range = dim->getHeightRange();
        auto  pos   = ctx.origin.getBlockPosition();
        pos.x       = (pos.x >> 4) << 4;
        pos.z       = (pos.z >> 4) << 4;
        pos.y       = range.min;

        pctx->region = Region::create(
            RegionType::Cuboid,
            dim->getDimensionId(),
            {
                pos,
                {pos.x + 15, range.max - 1, pos.z + 15}
        }
        );
        pctx->regionType = RegionType::Cuboid;
        ctx.success("chunk selected");
    });
};
} // namespace we
