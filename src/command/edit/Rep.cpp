#include "command/edit/CommandHelpers.h"

namespace we {
REG_CMD(edit, rep, "Replace matching blocks in the region") {
    struct Params {
        CommandBlockName                      blockBefore{};
        std::vector<::BlockStateCommandParam> beforeStates;
        CommandBlockName                      block{};
        CommandRawText                        pattern;
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
        auto beforeStates = BlockStateCommandParam::toStateMap(params.beforeStates);
        if (!beforeStates) {
            beforeStates.error().log(ctx.error());
            return;
        }
        auto& blockSource = dim->getBlockSourceFromMainChunkSource();

        auto beforeName = HashedString{params.blockBefore.getBlockName()};

        auto record = std::make_shared<HistoryRecord>();
        for (auto const& pos : region->forEachBlockInRegion()) {
            auto& current = blockSource.getBlock(pos);
            if (current.getBlockType().mNameInfo->mFullName->getHash()
                != beforeName.getHash()) {
                continue;
            }
            if (!beforeStates->empty()) {
                if (!current.mSerializationId->contains("states", Tag::Type::Compound)) {
                    continue;
                }
                auto& states = current.mSerializationId->at("states").get<CompoundTag>();
                bool  mismatch = false;
                for (auto& [name, val] : *beforeStates) {
                    if (!states.contains(name)) {
                        mismatch = true;
                        break;
                    }
                    CompoundTagVariant v{};
                    std::visit([&](auto& p) { v = p; }, val);
                    if (states.at(name) != v) {
                        mismatch = true;
                        break;
                    }
                }
                if (mismatch) continue;
            }
            record->record(*lctx, blockSource, {pos, pattern->pickBlock(pos)});
        }

        auto changed = record->apply(*lctx, blockSource);
        lctx->history.addRecord(std::move(record));
        if (changed == 0) {
            ctx.success("No blocks were changed");
        } else {
            ctx.success("Replaced {0} blocks", changed);
        }
    };

    command.overload<Params>()
        .required("blockBefore")
        .required("beforeStates")
        .required("block")
        .execute(CmdCtxBuilder{} | executor);
    command.overload<Params>()
        .required("blockBefore")
        .required("beforeStates")
        .required("pattern")
        .execute(CmdCtxBuilder{} | executor);
    command.overload<Params>()
        .required("blockBefore")
        .required("block")
        .execute(CmdCtxBuilder{} | executor);
}
} // namespace we
