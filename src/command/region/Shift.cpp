#include "command/CommandMacro.h"
#include "utils/FacingUtils.h"

namespace we {
REG_CMD(region, shift, "shift region") {
    struct Params {
        int           dis{};
        CommandFacing facing{CommandFacing::Me};
    };
    command.overload<Params>().required("dis").optional("facing").execute(
        CmdCtxBuilder{} |
        [](CommandContextRef const& ctx, Params const& params) {
            auto region = checkRegion(ctx);
            if (!region) return;
            auto facing = checkFacing(params.facing, ctx);
            if (!facing) return;
            auto res = region->shift(facingToDir(*facing, params.dis));
            if (res) {
                ctx.success("region shifted successfully");
            } else {
                ctx.error("region shifted failed");
            }
        }
    );
}
} // namespace we
