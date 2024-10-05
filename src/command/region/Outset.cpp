#include "command/CommandMacro.h"
#include "utils/FacingUtils.h"

namespace we {
REG_CMD(region, outset, "expand region on all axes") {
    struct Params {
        int                                dis{};
        ll::command::Optional<std::string> args; // TODO: hv
    };
    command.overload<Params>().required("dis").optional("args").execute(
        CmdCtxBuilder{} |
        [](CommandContextRef const& ctx, Params const& params) {
            auto region = checkRegion(ctx);
            if (!region) return;
            inplace_vector<BlockPos, 6> transformer;
            transformer.emplace_back(-params.dis, 0, 0);
            transformer.emplace_back(params.dis, 0, 0);
            transformer.emplace_back(0, -params.dis, 0);
            transformer.emplace_back(0, params.dis, 0);
            transformer.emplace_back(0, 0, -params.dis);
            transformer.emplace_back(0, 0, params.dis);
            auto res = region->expand(transformer);
            if (res) {
                ctx.success("region outseted successfully");
            } else {
                ctx.error("region outseted failed");
            }
        }
    );
};
} // namespace we
