#include "pattern/BlockListPattern.h"

#include "expression/expression.h"
#include "utils/BlockUtils.h"

#include <cmath>
#include <ll/api/utils/RandomUtils.h>

namespace we {
struct PatternCompiledExpr {
    bool   literalMode = true;
    double literal     = 0.0;

    mutable double x = 0.0;
    mutable double y = 0.0;
    mutable double z = 0.0;

    SymbolTable symbolTable;
    Expression  expression;

    explicit PatternCompiledExpr(double value) : literal(value) {}
    PatternCompiledExpr() = default;

    ll::Expected<> compile(std::string const& source) {
        literalMode = false;
        symbolTable.add_variable("x", x);
        symbolTable.add_variable("y", y);
        symbolTable.add_variable("z", z);
        symbolTable.add_constants();
        expression.register_symbol_table(symbolTable);

        Parser parser;
        if (!parser.compile(source, expression)) {
            ll::Error err;
            for (size_t i = 0; i < parser.error_count(); ++i) {
                auto const& error = parser.get_error(i);
                err.join(
                    ll::makeStringError(
                        fmt::format(
                            "Expression error at '{}': {}",
                            error.token.value,
                            error.diagnostic
                        )
                    )
                );
            }
            return ll::forwardError(err);
        }
        return {};
    }

    void updateVariables(BlockPos const& pos) const {
        if (literalMode) {
            return;
        }
        x = static_cast<double>(pos.x);
        y = static_cast<double>(pos.y);
        z = static_cast<double>(pos.z);
    }

    double evalScalar() const {
        if (literalMode) {
            return literal;
        }
        return expression.value();
    }

    bool evalString(std::string_view& out) const {
        if (literalMode) {
            return false;
        }

        expression.value();
        auto const& results = expression.results();
        for (size_t index = 0; index < results.count(); ++index) {
            auto& t = results[index];
            if (t.type == exprtk::type_store<double>::e_string) {
                const auto str = typename exprtk::type_store<double>::string_view(t);
                out            = std::string_view(str.begin(), str.size());
                return true;
            }
        }
        return false;
    }
};

static std::string_view trimBlockResult(std::string_view value) {
    size_t begin = 0;
    size_t end   = value.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(begin, end - begin);
}

static ll::Expected<std::unique_ptr<PatternCompiledExpr>>
compileExpr(PatternExpr const& expr) {
    if (auto literal = std::get_if<PatternLiteralExpr>(&expr)) {
        return std::make_unique<PatternCompiledExpr>(literal->value);
    }
    auto compiled = std::make_unique<PatternCompiledExpr>();
    auto result   = compiled->compile(std::get<PatternRuntimeExpr>(expr).source);
    if (!result) {
        return ll::forwardError(result.error());
    }
    return compiled;
}

static std::pair<std::string, Block::BlockStateValueType>
compileState(PatternBlockStateAst const& state) {
    return {
        state.key,
        std::visit(
            [](auto const& value) -> Block::BlockStateValueType { return value; },
            state.value
        )
    };
}

static ll::Expected<PatternCachedSimpleBlock>
makeCachedSimpleBlock(optional_ref<Block const> block, std::string const& description) {
    if (!block) {
        return ll::makeStringError(
            fmt::format("failed to resolve block: {}", description)
        );
    }
    return PatternCachedSimpleBlock{block};
}

static ll::Expected<PatternCompiledSimpleBlock>
compileSimpleBlock(PatternSimpleBlockSpec const& spec) {
    if (auto named = std::get_if<PatternNamedBlockAst>(&spec)) {
        if (!named->data) {
            if (!named->states.empty()) {
                std::vector<std::pair<std::string, Block::BlockStateValueType>>
                    blockStates;
                blockStates.reserve(named->states.size());
                for (auto const& state : named->states) {
                    blockStates.push_back(compileState(state));
                }
                return makeCachedSimpleBlock(
                    Block::tryGetFromRegistry(named->name, blockStates),
                    fmt::format("{} with states", named->name)
                );
            }
            return makeCachedSimpleBlock(
                Block::tryGetFromRegistry(named->name),
                named->name
            );
        }

        if (!named->states.empty()) {
            return ll::makeStringError(
                "block states cannot be combined with dynamic data"
            );
        }

        PatternDynamicSimpleBlock compiled;
        compiled.blockName = named->name;
        if (named->data) {
            auto data = compileExpr(*named->data);
            if (!data) {
                return ll::forwardError(data.error());
            }
            compiled.data = std::move(*data);
        }
        return compiled;
    }
    if (auto expr = std::get_if<PatternDynamicBlockAst>(&spec)) {
        PatternDynamicSimpleBlock compiled;
        auto                      blockExpr = compileExpr(expr->source);
        if (!blockExpr) {
            return ll::forwardError(blockExpr.error());
        }
        compiled.blockExpr = std::move(*blockExpr);
        if (expr->data) {
            auto data = compileExpr(*expr->data);
            if (!data) {
                return ll::forwardError(data.error());
            }
            compiled.data = std::move(*data);
        }
        return compiled;
    }
    auto tag = CompoundTag::fromSnbt(std::get<PatternSnbtBlockAst>(spec).source);
    if (!tag) {
        return ll::forwardError(tag.error());
    }
    return makeCachedSimpleBlock(
        Block::tryGetFromRegistry(*tag),
        std::get<PatternSnbtBlockAst>(spec).source
    );
}

static ll::Expected<PatternCachedSimpleBlock>
resolveStaticSimpleBlock(PatternDynamicSimpleBlock const& block) {
    if (block.blockExpr || block.data) {
        return ll::makeStringError("block is dynamic and cannot be cached");
    }
    if (block.blockName) {
        return makeCachedSimpleBlock(
            Block::tryGetFromRegistry(*block.blockName),
            *block.blockName
        );
    }
    return ll::makeStringError("block is empty and cannot be cached");
}

static optional_ref<Block const>
resolveSimpleBlock(PatternCompiledSimpleBlock const& block, BlockPos const& pos) {
    if (auto cached = std::get_if<PatternCachedSimpleBlock>(&block)) {
        return cached->block;
    }

    auto const& dynamic = std::get<PatternDynamicSimpleBlock>(block);

    std::optional<ushort> dataValue;
    if (dynamic.data) {
        dynamic.data->updateVariables(pos);
        auto dataEval = std::llround(dynamic.data->evalScalar());
        dataValue     = static_cast<ushort>(dataEval < 0 ? 0 : dataEval);
    }

    std::string_view dynamicName;
    if (dynamic.blockName) {
        if (dataValue) {
            return Block::tryGetFromRegistry(*dynamic.blockName, *dataValue);
        }
        return Block::tryGetFromRegistry(*dynamic.blockName);
    }

    if (!dynamic.blockExpr) {
        return nullptr;
    }
    dynamic.blockExpr->updateVariables(pos);
    if (!dynamic.blockExpr->evalString(dynamicName)) {
        return nullptr;
    }
    dynamicName = trimBlockResult(dynamicName);
    if (!dynamicName.empty() && dynamicName.front() == '{') {
        auto tag = CompoundTag::fromSnbt(dynamicName);
        if (!tag) {
            return nullptr;
        }
        return Block::tryGetFromRegistry(*tag);
    }
    if (dataValue) {
        return Block::tryGetFromRegistry(dynamicName, *dataValue);
    }
    return Block::tryGetFromRegistry(dynamicName);
}

static BlockOperation
resolveEntry(PatternCompiledEntry const& entry, BlockPos const& pos) {
    BlockOperation operation;

    if (entry.block) {
        operation.block = resolveSimpleBlock(*entry.block, pos);
    }
    if (entry.extraBlock) {
        operation.extraBlock = resolveSimpleBlock(*entry.extraBlock, pos);
    }
    if (entry.cachedBlockEntityTag) {
        operation.blockActor = BlockActor::create(*entry.cachedBlockEntityTag, pos);
    }
    return operation;
}

BlockListPattern::BlockListPattern(std::vector<PatternCompiledEntry> entries)
: entries(std::move(entries)) {}

ll::Expected<std::shared_ptr<BlockListPattern>>
BlockListPattern::create(BlockListPatternAst const& ast) {
    std::vector<PatternCompiledEntry> compiledEntries;
    compiledEntries.reserve(ast.entries.size());
    for (auto const& entry : ast.entries) {
        PatternCompiledEntry compiled;

        auto weight = compileExpr(entry.weight);
        if (!weight) {
            return ll::forwardError(weight.error());
        }
        compiled.weight = std::move(*weight);

        if (auto simple = std::get_if<PatternSimpleBlockSpec>(&entry.block.source)) {
            auto compiledBlock = compileSimpleBlock(*simple);
            if (!compiledBlock) {
                return ll::forwardError(compiledBlock.error());
            }
            if (auto dynamicBlock =
                    std::get_if<PatternDynamicSimpleBlock>(&compiledBlock.value())) {
                if (!dynamicBlock->blockExpr && !dynamicBlock->data) {
                    auto cachedBlock = resolveStaticSimpleBlock(*dynamicBlock);
                    if (!cachedBlock) {
                        return ll::forwardError(cachedBlock.error());
                    }
                    compiled.block =
                        PatternCompiledSimpleBlock{std::move(cachedBlock.value())};
                } else {
                    compiled.block = std::move(compiledBlock.value());
                }
            } else {
                compiled.block = std::move(compiledBlock.value());
            }
        } else {
            auto const& packed = std::get<PatternPackedBlockSpec>(entry.block.source);
            if (packed.block) {
                auto compiledBlock = compileSimpleBlock(*packed.block);
                if (!compiledBlock) {
                    return ll::forwardError(compiledBlock.error());
                }
                if (auto dynamicBlock =
                        std::get_if<PatternDynamicSimpleBlock>(&compiledBlock.value())) {
                    if (!dynamicBlock->blockExpr && !dynamicBlock->data) {
                        auto cachedBlock = resolveStaticSimpleBlock(*dynamicBlock);
                        if (!cachedBlock) {
                            return ll::forwardError(cachedBlock.error());
                        }
                        compiled.block =
                            PatternCompiledSimpleBlock{std::move(cachedBlock.value())};
                    } else {
                        compiled.block = std::move(compiledBlock.value());
                    }
                } else {
                    compiled.block = std::move(compiledBlock.value());
                }
            }
            if (packed.extraBlock) {
                auto compiledExtra = compileSimpleBlock(*packed.extraBlock);
                if (!compiledExtra) {
                    return ll::forwardError(compiledExtra.error());
                }
                if (auto dynamicExtra =
                        std::get_if<PatternDynamicSimpleBlock>(&compiledExtra.value())) {
                    if (!dynamicExtra->blockExpr && !dynamicExtra->data) {
                        auto cachedExtra = resolveStaticSimpleBlock(*dynamicExtra);
                        if (!cachedExtra) {
                            return ll::forwardError(cachedExtra.error());
                        }
                        compiled.extraBlock =
                            PatternCompiledSimpleBlock{std::move(cachedExtra.value())};
                    } else {
                        compiled.extraBlock = std::move(compiledExtra.value());
                    }
                } else {
                    compiled.extraBlock = std::move(compiledExtra.value());
                }
            }
            if (packed.blockEntity) {
                auto blockEntityTag = CompoundTag::fromSnbt(*packed.blockEntity);
                if (!blockEntityTag) {
                    return ll::forwardError(blockEntityTag.error());
                }
                compiled.cachedBlockEntityTag = std::move(*blockEntityTag);
            }
        }

        compiledEntries.push_back(std::move(compiled));
    }
    return std::make_shared<BlockListPattern>(std::move(compiledEntries));
}

BlockOperation BlockListPattern::pickBlock(BlockPos const& pos) const {
    if (entries.empty()) {
        return {};
    }

    std::vector<double> weights;
    weights.reserve(entries.size());
    double total = 0.0;
    for (auto const& entry : entries) {
        entry.weight->updateVariables(pos);
        auto weight = std::max(0.0, entry.weight->evalScalar());
        weights.push_back(weight);
        total += weight;
    }
    if (total <= 0.0) {
        return {};
    }

    auto   target = ll::utils::random_utils::rand<double>(total);
    double sum    = 0.0;
    for (size_t i = 0; i < entries.size(); ++i) {
        sum += weights[i];
        if (target <= sum || i + 1 == entries.size()) {
            return resolveEntry(entries[i], pos);
        }
    }
    return {};
}
} // namespace we
