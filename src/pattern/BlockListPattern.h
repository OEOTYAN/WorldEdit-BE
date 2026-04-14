#pragma once

#include "pattern/Pattern.h"

#include "expression/expression.h"

#include <memory>

namespace we {
struct PatternCachedSimpleBlock {
    optional_ref<Block const> block = nullptr;
};

struct PatternDynamicSimpleBlock {
    std::optional<std::string>    blockName;
    std::unique_ptr<CompiledExpr> data;
    std::unique_ptr<CompiledExpr> blockExpr;
};

using PatternCompiledSimpleBlock =
    std::variant<PatternCachedSimpleBlock, PatternDynamicSimpleBlock>;

struct PatternCompiledEntry {
    std::unique_ptr<CompiledExpr>             weight;
    std::optional<PatternCompiledSimpleBlock> block;
    std::optional<PatternCompiledSimpleBlock> extraBlock;
    std::optional<CompoundTag>                cachedBlockEntityTag;
};

class BlockListPattern : public Pattern {
    std::vector<PatternCompiledEntry> entries;
    std::shared_ptr<ExprState>        exprState;

public:
    explicit BlockListPattern(
        std::vector<PatternCompiledEntry> entries,
        std::shared_ptr<ExprState>        exprState
    );

    static ll::Expected<std::shared_ptr<BlockListPattern>>
    create(BlockListPatternAst const& ast);

    ll::Expected<> prepare(CommandContextRef const& ctx) override;

    BlockOperation pickBlock(BlockPos const& pos) const override;
};
} // namespace we
