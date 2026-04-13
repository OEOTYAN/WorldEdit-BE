#pragma once

#include "pattern/PatternAst.h"
#include "worldedit/Global.h"

namespace we {
class CommandContextRef;

class Pattern {
public:
    virtual ~Pattern() = default;

    virtual ll::Expected<> prepare(CommandContextRef const&) { return {}; }

    virtual optional_ref<Block const> pickBlock(BlockPos const& pos) const = 0;

    static ll::Expected<std::shared_ptr<Pattern>> fromAst(PatternAst const& ast);
};
} // namespace we
