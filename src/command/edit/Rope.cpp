#include "command/edit/CommandHelpers.h"
#include "utils/Math.h"

namespace we {

REG_CMD(edit, rope, "Draw catenary curves between region vertices") {
    struct Params {
        CommandBlockName block{};
        CommandRawText   pattern;
        float            extraLengthPercentage = 20.0f;
        float            radius                = 0.0f;
        struct VaArgs {
            bool hollow = false;
        } args;
    };

    auto executor = [](CommandContextRef const& ctx, Params const& params) {
        auto lctx = checkLocalContext(ctx);
        if (!lctx) return;
        auto region = checkRegion(ctx);
        if (!region) return;
        auto dim = checkDimension(ctx);
        if (!dim) return;
        auto pattern = preparePattern(ctx, params.block, params.pattern);
        if (!pattern) return;

        auto type = region->getType();
        if (type != RegionType::Cuboid && type != RegionType::Convex
            && type != RegionType::Loft) {
            ctx.error("Rope only supports cuboid, convex and loft regions");
            return;
        }

        auto& blockSource = dim->getBlockSourceFromMainChunkSource();

        double extraLength = 1.0 + params.extraLengthPercentage / 100.0;
        int    radius      = static_cast<int>(params.radius);
        ll::SmallDenseSet<BlockPos> tmp;

        for (auto const& [pos1, pos2] : region->forEachLine()) {
            if (extraLength > 1.0 && (pos1.x != pos2.x || pos1.z != pos2.z)) {
                double d   = pos1.distanceTo(pos2);
                double dh  = std::sqrt(pow2(pos1.x - pos2.x) + pow2(pos1.z - pos2.z));
                double h   = pos2.y - pos1.y;
                double L   = d * extraLength;
                double a   = getCatenaryParameter(dh, h, L);
                double x0  = (a * std::log((L - h) / (L + h)) - dh) * 0.5;
                double y0  = a * std::cosh(x0 / a);
                int    dx  = pos2.x - pos1.x;
                int    dz  = pos2.z - pos1.z;
                x0        += dh;
                y0        -= h;
                double df  = 1.0 / (10.0 * L);
                for (double i = 0.0; i <= 1.0; i += df) {
                    double kx, kz;
                    double ky  = a * std::cosh((i * dh - x0) / a) - y0;
                    ky        += pos1.y;
                    kx         = pos1.x + dx * i;
                    kz         = pos1.z + dz * i;
                    BlockPos kpos{
                        static_cast<int>(std::floor(kx + 0.5)),
                        static_cast<int>(std::floor(ky + 0.5)),
                        static_cast<int>(std::floor(kz + 0.5))
                    };
                    if (params.radius == 0) {
                        tmp.insert(kpos);
                    } else {
                        auto box = BoundingBox(kpos - (radius + 1), kpos + (radius + 1));
                        for (auto const& posk : box.forEachPos()) {
                            if (std::sqrt(
                                    pow2(posk.x - kx) + pow2(posk.y - ky)
                                    + pow2(posk.z - kz)
                                )
                                <= 0.5 + params.radius) {
                                tmp.insert(posk);
                            }
                        }
                    }
                }
            } else {
                for (auto const& pos : plotLine(pos1, pos2)) {
                    auto box = BoundingBox(pos - (radius + 1), pos + (radius + 1));
                    for (auto const& posk : box.forEachPos()) {
                        if ((pos - posk).length() <= 0.5f + params.radius) {
                            tmp.insert(posk);
                        }
                    }
                }
            }
        }

        if (params.args.hollow) {
            ll::SmallDenseSet<BlockPos> tmp2;
            for (auto const& pos : tmp) {
                if (countNeighbors6InSet(pos, tmp) < 6) {
                    tmp2.insert(pos);
                }
            }
            tmp = std::move(tmp2);
        }

        auto record = std::make_shared<HistoryRecord>();
        for (auto const& pos : tmp) {
            record->record(*lctx, blockSource, {pos, pattern->pickBlock(pos)});
        }

        auto changed = record->apply(*lctx, blockSource);
        lctx->history.addRecord(std::move(record));
        if (changed == 0) {
            ctx.success("No blocks were changed");
        } else {
            ctx.success("Drew {0} rope block(s)", changed);
        }
    };

    command.overload<Params>()
        .required("block")
        .optional("extraLengthPercentage")
        .optional("radius")
        .optional("args")
        .execute(CmdCtxBuilder{} | executor);
    command.overload<Params>()
        .required("pattern")
        .optional("extraLengthPercentage")
        .optional("radius")
        .optional("args")
        .execute(CmdCtxBuilder{} | executor);
}
} // namespace we
