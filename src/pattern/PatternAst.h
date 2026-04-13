#pragma once

#include "worldedit/Global.h"

#include <optional>
#include <variant>

namespace we {
struct PatternLiteralExpr {
    double value = 0.0;
};

struct PatternRuntimeExpr {
    std::string source;
};

using PatternExpr = std::variant<PatternLiteralExpr, PatternRuntimeExpr>;

struct PatternBlockSpec {
    std::variant<std::string, PatternRuntimeExpr> source;
};

struct PatternWeightedEntry {
    PatternExpr                weight;
    PatternBlockSpec           block;
    std::optional<PatternExpr> data;
};

struct HandPatternAst {};

struct BlockListPatternAst {
    std::vector<PatternWeightedEntry> entries;
};

using PatternAst = std::variant<HandPatternAst, BlockListPatternAst>;
} // namespace we
