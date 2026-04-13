#pragma once

#include "data/Operation.h"
#include "pattern/PatternAst.h"
#include "worldedit/Global.h"

namespace we {
class CommandContextRef;

class Pattern {
public:
    virtual ~Pattern() = default;

    virtual ll::Expected<> prepare(CommandContextRef const&) { return {}; }

    virtual BlockOperation pickBlock(BlockPos const& pos) const = 0;

    static ll::Expected<std::shared_ptr<Pattern>> fromAst(PatternAst const& ast);
};
} // namespace we
