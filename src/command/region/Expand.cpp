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
                if (bool res = region->expand(transformer); res) {
                    ctx.success("region expanded successfully");
                } else {
                    ctx.error("region expanded failed");
                }
            }
        );
    struct ToParams {
        ll::command::Optional<CommandPositionFloat> pos;
        CommandFacing                               facing{CommandFacing::Me};
    };
    command.overload<ToParams>().text("to").optional("pos").optional("facing").execute(
        CmdCtxBuilder{} |
        [](CommandContextRef const& ctx, ToParams const& params) {
            auto region = checkRegion(ctx);
            if (!region) return;
            auto facing = checkFacing(params.facing, ctx);
            if (!facing) return;
            BlockPos pos = params.pos.transform(ctx.transformPos()).value_or(ctx.pos());
            auto     box = region->getBoundingBox();
            switch (*facing) {
            case FacingID::Down:
                if (int length = box.min.y - pos.y; length > 0) {
                    pos = {0, -length, 0};
                } else {
                    pos = 0;
                }
                break;
            case FacingID::Up:
                if (int length = pos.y - box.max.y; length > 0) {
                    pos = {0, length, 0};
                } else {
                    pos = 0;
                }
                break;
            case FacingID::North:
                if (int length = box.min.z - pos.z; length > 0) {
                    pos = {0, 0, -length};
                } else {
                    pos = 0;
                }
                break;
            case FacingID::South:
                if (int length = pos.z - box.max.z; length > 0) {
                    pos = {0, 0, length};
                } else {
                    pos = 0;
                }
                break;
            case FacingID::West:
                if (int length = box.min.x - pos.x; length > 0) {
                    pos = {-length, 0, 0};
                } else {
                    pos = 0;
                }
                break;
            case FacingID::East:
                if (int length = pos.x - box.max.x; length > 0) {
                    pos = {length, 0, 0};
                } else {
                    pos = 0;
                }
                break;
            default:
                pos = 0;
            }
            if (pos == 0) {
                ctx.error("region has already attached the position");
                return;
            }
            if (bool res = region->expand({&pos, 1}); res) {
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
                .emplace_back(0, std::min(heightRange.mMin - boundingBox.min.y, 0), 0);
            transformer
                .emplace_back(0, std::max(heightRange.mMax - boundingBox.max.y - 1, 0), 0);

            if (bool res = region->expand(transformer); res) {
                ctx.success("region expanded successfully");
            } else {
                ctx.error("region expanded failed");
            }
        }
    );
};
} // namespace we
