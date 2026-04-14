#pragma once

#include "pattern/Pattern.h"

#include <memory>

namespace we {
struct PatternCompiledExpr;
struct PatternCachedSimpleBlock {
    optional_ref<Block const> block = nullptr;
};

struct PatternDynamicSimpleBlock {
    std::optional<std::string>           blockName;
    std::unique_ptr<PatternCompiledExpr> data;
    std::unique_ptr<PatternCompiledExpr> blockExpr;
};

using PatternCompiledSimpleBlock =
    std::variant<PatternCachedSimpleBlock, PatternDynamicSimpleBlock>;

struct PatternCompiledEntry {
    std::unique_ptr<PatternCompiledExpr>      weight;
    std::optional<PatternCompiledSimpleBlock> block;
    std::optional<PatternCompiledSimpleBlock> extraBlock;
    std::optional<CompoundTag>                cachedBlockEntityTag;
};

class BlockListPattern : public Pattern {
    std::vector<PatternCompiledEntry> entries;

public:
    explicit BlockListPattern(std::vector<PatternCompiledEntry> entries);

    static ll::Expected<std::shared_ptr<BlockListPattern>>
    create(BlockListPatternAst const& ast);

    BlockOperation pickBlock(BlockPos const& pos) const override;
};
} // namespace we
