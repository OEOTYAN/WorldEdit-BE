#include "command/CommandMacro.h"
#include "utils/FacingUtils.h"

namespace we {
REG_CMD(region, contract, "contract region") {
    struct Params {
        int           dis{};
        CommandFacing facing{CommandFacing::Me};
        int           disBehind{};
    };
    command.overload<Params>()
        .required("dis")
        .optional("facing")
        .optional("disBehind")
        .execute(
            CmdCtxBuilder{} |
            [](CommandContextRef const& ctx, Params const& params) {
                auto region = checkRegion(ctx);
                if (!region) return;
                auto facing = checkFacing(params.facing, ctx);
                if (!facing) return;
                inplace_vector<BlockPos, 2> transformer;
                transformer.emplace_back(facingToDir(*facing, params.dis));
                if (params.disBehind) {
                    transformer.emplace_back(
                        facingToDir(opposite(*facing), params.disBehind)
                    );
                }
                auto res = region->contract(transformer);
                if (res) {
                    ctx.success("region contracted successfully");
                } else {
                    ctx.error("region contracted failed");
                }
            }
        );
};
} // namespace we
