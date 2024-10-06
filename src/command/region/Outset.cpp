#include "command/CommandMacro.h"
#include "utils/FacingUtils.h"

namespace we {
REG_CMD(region, outset, "expand region on all axes") {
    struct Params {
        int dis{};
        struct VaArgs {
            bool horizontal{true};
            bool vertical{true};
        } args;
    };
    command.overload<Params>().required("dis").optional("args").execute(
        CmdCtxBuilder{} |
        [](CommandContextRef const& ctx, Params const& params) {
            auto region = checkRegion(ctx);
            if (!region) return;
            inplace_vector<BlockPos, 6> transformer;
            if (params.args.horizontal) {
                transformer.emplace_back(-params.dis, 0, 0);
                transformer.emplace_back(params.dis, 0, 0);
                transformer.emplace_back(0, 0, -params.dis);
                transformer.emplace_back(0, 0, params.dis);
            }
            if (params.args.vertical) {
                transformer.emplace_back(0, -params.dis, 0);
                transformer.emplace_back(0, params.dis, 0);
            }
            auto res = region->expand(transformer);
            if (res) {
                ctx.success("region expanded successfully");
            } else {
                ctx.error("region expanded failed");
            }
        }
    );
};
} // namespace we
