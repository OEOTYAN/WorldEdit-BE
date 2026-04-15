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

using PatternStateValue = std::variant<int, float, bool, std::string>;

struct PatternBlockStateAst {
    std::string       key;
    PatternStateValue value;
};

struct PatternNamedBlockAst {
    std::string                       name;
    std::vector<PatternBlockStateAst> states;
    std::optional<PatternExpr>        data;
};

struct PatternDynamicBlockAst {
    PatternExpr                source;
    std::optional<PatternExpr> data;
};

struct PatternSnbtBlockAst {
    std::string source;
};

using PatternSimpleBlockSpec =
    std::variant<PatternNamedBlockAst, PatternDynamicBlockAst, PatternSnbtBlockAst>;

struct PatternPackedBlockSpec {
    std::optional<PatternSimpleBlockSpec> block;
    std::optional<PatternSimpleBlockSpec> extraBlock;
    std::optional<std::string>            blockEntity;
};

struct PatternBlockSpec {
    std::variant<PatternSimpleBlockSpec, PatternPackedBlockSpec> source;
};

struct PatternWeightedEntry {
    PatternExpr      weight;
    PatternBlockSpec block;
};

struct HandPatternAst {};

struct ClipboardPatternAst {
    BlockPos offset{0, 0, 0};
    bool     useCenter = false;
};

struct GradientPatternAst {
    bool                     lighten = false;
    std::vector<std::string> selectors;
};

struct BlockListPatternAst {
    std::vector<PatternWeightedEntry> entries;
};

using PatternAst = std::
    variant<HandPatternAst, ClipboardPatternAst, GradientPatternAst, BlockListPatternAst>;
} // namespace we
