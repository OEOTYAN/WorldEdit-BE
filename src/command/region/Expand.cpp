#include "command/CommandMacro.h"
#include "utils/FacingUtils.h"

namespace we {
REG_CMD(region, expand, "expand region") {
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
                auto res = region->expand(transformer);
                if (res) {
                    ctx.success("region expanded successfully");
                } else {
                    ctx.error("region expanded failed");
                }
            }
        );
    command.overload().text("vert").execute(
        CmdCtxBuilder{} |
        [](CommandContextRef const& ctx) {
            auto region = checkRegion(ctx);
            if (!region) return;
            auto dim = checkDimension(ctx);
            if (!dim) return;

            inplace_vector<BlockPos, 2> transformer;
            auto                        heightRange = dim->getHeightRange();
            auto                        boundingBox = region->getBoundingBox();
            transformer
                .emplace_back(0, std::min(heightRange.min - boundingBox.min.y, 0), 0);
            transformer
                .emplace_back(0, std::max(heightRange.max - boundingBox.max.y - 1, 0), 0);
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
