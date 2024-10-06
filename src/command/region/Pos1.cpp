#include "command/CommandMacro.h"

namespace we {
REG_CMD(region, pos1, "sets the main position to the given or current position") {
    struct Params {
        ll::command::Optional<CommandPositionFloat> pos;
    };
    command.overload<Params>().optional("pos").execute(
        CmdCtxBuilder{} |
        [](CommandContextRef const& ctx, Params const& params) {
            auto dim = checkDimension(ctx);
            if (!dim) return;
            BlockPos pos = params.pos.transform(ctx.transformPos()).value_or(ctx.pos());
            if (getLocalContext(ctx)->setMainPos({pos, dim->getDimensionId()})) {
                ctx.success("set main position at {0}", pos);
            } else {
                ctx.error("can't set main position at {0}", pos);
            }
        }
    );
};
} // namespace we
