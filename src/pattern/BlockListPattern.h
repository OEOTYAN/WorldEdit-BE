#pragma once

#include "pattern/Pattern.h"

#include <memory>

namespace we {
struct PatternCompiledExpr;
struct PatternCompiledEntry {
    std::unique_ptr<PatternCompiledExpr> weight;
    std::optional<std::string>           blockName;
    std::unique_ptr<PatternCompiledExpr> blockExpr;
    std::unique_ptr<PatternCompiledExpr> data;
};

class BlockListPattern : public Pattern {
    std::vector<PatternCompiledEntry> entries;

public:
    explicit BlockListPattern(std::vector<PatternCompiledEntry> entries);

    static ll::Expected<std::shared_ptr<BlockListPattern>>
    create(BlockListPatternAst const& ast);

    optional_ref<Block const> pickBlock(BlockPos const& pos) const override;
};
} // namespace we
