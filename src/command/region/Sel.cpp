#include "command/CommandMacro.h"

namespace we {
REG_CMD(region, sel, "manipulate region") {
    struct Sel {
        RegionType type;
    };
    enum class SelRemoveType {
        Last,
        Near,
    };
    struct SelRm {
        SelRemoveType                               type;
        ll::command::Optional<CommandPositionFloat> pos;
    };
    command.overload().text("clear").execute(
        CmdCtxBuilder{} |
        [](CommandContextRef const& ctx) {
            auto pctx = checkPlayerContext(ctx);
            if (!pctx) return;
            pctx->region.reset();
            pctx->mainPos.reset();
            pctx->offPos.reset();
            ctx.success("region cleared");
        }
    );
    command.overload<SelRm>().text("remove").required("type").optional("pos").execute(
        CmdCtxBuilder{} |
        [](CommandContextRef const& ctx, SelRm const& params) {
            auto dim = checkDimension(ctx);
            if (!dim) return;
            auto region = checkRegion(ctx);
            if (!region) return;
            if (dim->getDimensionId() != region->getDim()) {
                ctx.error("origin dimension doesn't match region dimension");
                return;
            }
            bool res;
            if (params.type == SelRemoveType::Last) {
                res = region->removePoint(std::nullopt);
            } else {
                res = region->removePoint(
                    params.pos.transform(ctx.transformPos()).value_or(ctx.pos())
                );
            }
            if (res) {
                ctx.success("selected postion removed");
            } else {
                ctx.error("fail to remove selected postion");
            }
        }
    );
    command.overload<Sel>().required("type").execute(
        CmdCtxBuilder{} |
        [](CommandContextRef const& ctx, Sel const& params) {
            auto pctx = getPlayerContext(ctx);
            if (pctx->region) {
                pctx->region = Region::create(
                    params.type,
                    pctx->region->getDim(),
                    pctx->region->getBoundingBox()
                );
            }
            pctx->regionType = params.type;
            pctx->mainPos.reset();
            pctx->offPos.reset();
            ctx.success("region switch to {0}", params.type);
        }
    );
};
} // namespace we
