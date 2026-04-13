#pragma once

#include "data/ClipboardData.h"
#include "pattern/Pattern.h"

namespace we {
class ClipboardPattern : public Pattern {
    BlockPos                       offset;
    bool                           useCenter = false;
    BlockPos                       bias{0, 0, 0};
    std::shared_ptr<ClipboardData> clipboard;

public:
    ClipboardPattern(BlockPos const& offset, bool useCenter);

    static ll::Expected<std::shared_ptr<ClipboardPattern>>
    create(ClipboardPatternAst const& ast);

    ll::Expected<> prepare(CommandContextRef const& ctx) override;

    BlockOperation pickBlock(BlockPos const& pos) const override;
};
} // namespace we