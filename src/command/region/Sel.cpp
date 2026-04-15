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
        CmdCtxBuilder{} | [](CommandContextRef const& ctx) {
            auto lctx = checkLocalContext(ctx);
            if (!lctx) return;
            lctx->region.reset();
            lctx->mainPos.reset();
            lctx->offPos.reset();
            ctx.success("region cleared");
        }
    );
    command.overload().text("info").execute(
        CmdCtxBuilder{} | [](CommandContextRef const& ctx) {
            auto lctx = checkLocalContext(ctx);
            if (!lctx) return;
            auto region = checkRegion(ctx);
            if (!region) return;

            auto   box     = region->getBoundingBox();
            auto   boxSize = box.max - box.min + 1;
            auto   center  = region->getCenter();
            size_t actual  = 0;
            for (auto const& _ : region->forEachBlockInRegion()) {
                ++actual;
            }

            ctx.success("type: {}", magic_enum::enum_name(region->getType()));
            ctx.success("dimension: {}", region->getDim().id);
            ctx.success("min: {}", box.min);
            ctx.success("max: {}", box.max);
            ctx.success("center: {}", center);
            ctx.success("volume: {}", boxSize);
            ctx.success("approxsize: {}", region->size());
            ctx.success("actual-blocks: {}", actual);
            if (lctx->mainPos) {
                ctx.success("main: {}", lctx->mainPos->data.pos);
            }
            if (lctx->offPos) {
                ctx.success("off: {}", lctx->offPos->data.pos);
            }
            for (auto const& entry : region->getInfo()) {
                ctx.success("{}", entry);
            }
        }
    );
    command.overload<SelRm>().text("remove").required("type").optional("pos").execute(
        CmdCtxBuilder{} | [](CommandContextRef const& ctx, SelRm const& params) {
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
        CmdCtxBuilder{} | [](CommandContextRef const& ctx, Sel const& params) {
            auto lctx = getLocalContext(ctx);
            if (lctx->region) {
                lctx->region = Region::create(
                    params.type,
                    lctx->region->getDim(),
                    lctx->region->getBoundingBox()
                );
            }
            lctx->regionType = params.type;
            lctx->mainPos.reset();
            lctx->offPos.reset();
            ctx.success("region switch to {0}", params.type);
        }
    );
}
} // namespace we
