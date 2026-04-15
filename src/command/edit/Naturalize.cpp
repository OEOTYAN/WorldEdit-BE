#include "command/edit/CommandHelpers.h"

namespace we {
REG_CMD(edit, naturalize, "Replace exposed stone with grass and dirt") {
    auto executor = [](CommandContextRef const& ctx) {
        auto lctx = checkLocalContext(ctx);
        if (!lctx) return;
        auto region = checkRegion(ctx);
        if (!region) return;
        auto dim = checkDimension(ctx);
        if (!dim) return;
        auto& blockSource = dim->getBlockSourceFromMainChunkSource();

        auto const& stone =
            Block::tryGetFromRegistry(VanillaBlockTypeIds::Stone()).value();
        auto const& grass =
            Block::tryGetFromRegistry(VanillaBlockTypeIds::GrassBlock()).value();
        auto const& dirt = Block::tryGetFromRegistry(VanillaBlockTypeIds::Dirt()).value();

        auto record = std::make_shared<HistoryRecord>();

        auto bottom =
            std::max(region->getBoundingBox().min.y, (int)dim->mHeightRange->mMin);

        forEachTopBlock(*region, blockSource, [&](BlockPos pos) {
            while (pos.y >= bottom) {
                int dist = 0;
                while (pos.y >= bottom) {
                    if (blockSource.getBlock(pos).isAir()) {
                        pos.y--;
                        break;
                    }
                    dist++;
                    if (!region->contains(pos)) {
                        pos.y--;
                        continue;
                    }
                    BlockOperation op;
                    if (dist == 1) {
                        op.block = grass;
                    } else if (1 < dist && dist <= 4) {
                        op.block = dirt;
                    } else {
                        op.block = stone;
                    }
                    record->record(*lctx, blockSource, {pos, std::move(op)});
                    pos.y--;
                }
            }
        });
        auto changed = record->apply(*lctx, blockSource);
        lctx->history.addRecord(std::move(record));
        if (changed == 0) {
            ctx.success("No blocks were changed");
        } else {
            ctx.success("Naturalized {0} block(s)", changed);
        }
    };

    command.overload().execute(CmdCtxBuilder{} | executor);
}
} // namespace we
