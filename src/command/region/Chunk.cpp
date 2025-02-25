#include "command/CommandMacro.h"

namespace we {
REG_CMD(region, chunk, "select current chunk") {
    command.overload().execute(CmdCtxBuilder{} | [](CommandContextRef const& ctx) {
        auto dim = checkDimension(ctx);
        if (!dim) return;
        auto  lctx  = getLocalContext(ctx);
        auto& range = dim->mHeightRange.get();
        auto  pos   = ctx.origin.getBlockPosition();
        pos.x       = (pos.x >> 4) << 4;
        pos.z       = (pos.z >> 4) << 4;
        pos.y       = range.mMin;

        lctx->region = Region::create(
            RegionType::Cuboid,
            dim->getDimensionId(),
            {
                pos,
                {pos.x + 15, range.mMax - 1, pos.z + 15}
        }
        );
        lctx->regionType = RegionType::Cuboid;
        ctx.success("current chunk selected");
    });
};
} // namespace we
