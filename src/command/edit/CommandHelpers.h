#pragma once

#include "command/CommandMacro.h"
#include "data/HistoryRecord.h"
#include "pattern/Pattern.h"
#include "pattern/PatternParser.h"

namespace we {
inline std::shared_ptr<Pattern> preparePattern(
    CommandContextRef const& ctx,
    CommandBlockName const&  block,
    CommandRawText const&    pattern
) {
    std::string_view patternStr = pattern.mText;
    if (block.mBlockNameHash != 0) {
        patternStr = block.getBlockName();
    }

    auto patternAst = PatternParser::parse(patternStr);
    if (!patternAst) {
        patternAst.error().log(ctx.error(), ctx.localeCode());
        return nullptr;
    }
    auto parsedPattern = Pattern::fromAst(*patternAst);
    if (!parsedPattern) {
        parsedPattern.error().log(ctx.error(), ctx.localeCode());
        return nullptr;
    }
    auto prepared = (*parsedPattern)->prepare(ctx);
    if (!prepared) {
        prepared.error().log(ctx.error(), ctx.localeCode());
        return nullptr;
    }
    return *parsedPattern;
}
template <typename Visitor>
inline void forEachTopBlock(Region const& region, BlockSource& bs, Visitor&& visitor) {
    auto box = region.getBoundingBox();
    for (int x = box.min.x; x <= box.max.x; ++x) {
        for (int z = box.min.z; z <= box.max.z; ++z) {
            int top = region.getHeighest({x, z});
            while (top >= box.min.y&&bs.getBlock({x, top, z}).isAir()) top--;
            if (top < box.min.y) {
                continue;
            }
            BlockPos pos{x, top, z};
            if (region.contains(pos)) {
                visitor(pos);
            }
        }
    }
}

} // namespace we
