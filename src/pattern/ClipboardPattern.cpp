#include "pattern/ClipboardPattern.h"

#include "command/CommandMacro.h"

namespace we {
ClipboardPattern::ClipboardPattern(BlockPos const& offset, bool useCenter)
: offset(offset),
  useCenter(useCenter) {}

ll::Expected<std::shared_ptr<ClipboardPattern>>
ClipboardPattern::create(ClipboardPatternAst const& ast) {
    return std::make_shared<ClipboardPattern>(ast.offset, ast.useCenter);
}

ll::Expected<> ClipboardPattern::prepare(CommandContextRef const& ctx) {
    auto lctx = checkLocalContext(ctx);
    if (!lctx) {
        return ll::makeStringError("#clipboard requires LocalContext");
    }
    if (!lctx->clipboard || lctx->clipboard->empty()) {
        return ll::makeStringError("#clipboard requires non-empty clipboard");
    }

    clipboard = lctx->clipboard;

    if (auto region = checkRegion(ctx); region) {
        if (useCenter) {
            bias = BlockPos{region->getCenter()};
        } else {
            bias = region->getBoundingBox().min;
        }
    }
    bias -= offset;
    return {};
}

BlockOperation ClipboardPattern::pickBlock(BlockPos const& pos) const {
    if (!clipboard) {
        return {};
    }
    return clipboard->getBlock(pos - bias);
}
} // namespace we