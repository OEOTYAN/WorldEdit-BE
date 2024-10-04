#include "command/CommandMacro.h"
#include "region/LoftRegion.h"

namespace we {
struct Params {
    bool cycle;
};
REG_CMD(region, loftcycle, "set loft region curve is connected end to end") {
    command.overload<Params>().required("cycle").execute(
        CmdCtxBuilder{} |
        [](CommandContextRef const& ctx, Params const& params) {
            auto region = checkRegion(ctx);
            if (!region) return;
            if (region->getType() != RegionType::Loft) {
                ctx.error("origin didn't selected loft region");
                return;
            }
            std::static_pointer_cast<LoftRegion>(region)->setCycle(params.cycle);
            ctx.success("set loft region cycle to {0}", params.cycle);
        }
    );
};
} // namespace we
