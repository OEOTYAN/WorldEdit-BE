#include "command/CommandMacro.h"
#include "pattern/Pattern.h"
#include "pattern/PatternParser.h"

namespace we {
REG_CMD(edit, set, "Set blocks in the region to a specific block type") {
    struct Params {
        CommandBlockName block{};
        CommandRawText   pattern;
    };

    auto executor = [](CommandContextRef const& ctx, Params const& params) {
        auto lctx = checkLocalContext(ctx);
        if (!lctx) return;
        auto region = checkRegion(ctx);
        if (!region) return;
        auto dim = checkDimension(ctx);
        if (!dim) return;
        auto& blockSource = dim->getBlockSourceFromMainChunkSource();

        std::string_view patternStr = params.pattern.mText;
        if (params.block.mBlockNameHash != 0) {
            patternStr = params.block.getBlockName();
        }
        auto patternAst = PatternParser::parse(patternStr);
        if (!patternAst) {
            ctx.error("Invalid pattern");
            patternAst.error().log(ctx.output);
            return;
        }
        auto pattern = Pattern::fromAst(*patternAst);
        if (!pattern) {
            ctx.error("Failed to create pattern");
            pattern.error().log(ctx.output);
            return;
        }

        auto   record = std::make_shared<HistoryRecord>();
        size_t num    = 0;
        region->forEachBlockInRegion([&](BlockPos const& pos) {
            auto block = (*pattern)->pickBlock(pos);
            if (!block) {
                return;
            }
            Operation op;
            op.pos        = pos;
            auto& blockOp = op.operation.emplace<BlockOperation>();
            blockOp.block = block;
            record->record(*lctx, blockSource, op);
        });
        num = record->apply(*lctx, blockSource);
        lctx->history.addRecord(std::move(record));
        if (num == 0) {
            ctx.success("No blocks were changed");
        } else {
            ctx.success("Set {0} blocks", num);
        }
    };

    command.overload<Params>().required("block").execute(CmdCtxBuilder{} | executor);
    command.overload<Params>().required("pattern").execute(CmdCtxBuilder{} | executor);
}
} // namespace we
