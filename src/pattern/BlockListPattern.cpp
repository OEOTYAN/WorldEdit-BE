#include "pattern/BlockListPattern.h"

#include "command/CommandMacro.h"
#include "expression/expression.h"
#include "utils/BlockUtils.h"

#include <cmath>
#include <ll/api/utils/RandomUtils.h>
#include <utility>

namespace we {
static std::shared_ptr<ExprState> makeExprState(BlockListPatternAst const& ast) {
    bool needsState = false;

    auto inspectExpr = [&](PatternExpr const& expr) {
        needsState = needsState || std::holds_alternative<PatternRuntimeExpr>(expr);
    };

    for (auto const& entry : ast.entries) {
        inspectExpr(entry.weight);
        if (auto simple = std::get_if<PatternSimpleBlockSpec>(&entry.block.source)) {
            if (auto named = std::get_if<PatternNamedBlockAst>(simple)) {
                if (named->data) {
                    inspectExpr(*named->data);
                }
            } else if (auto dynamic = std::get_if<PatternDynamicBlockAst>(simple)) {
                inspectExpr(dynamic->source);
                if (dynamic->data) {
                    inspectExpr(*dynamic->data);
                }
            }
        } else {
            auto const& packed = std::get<PatternPackedBlockSpec>(entry.block.source);
            auto inspectSimple = [&](
                                     std::optional<PatternSimpleBlockSpec> const& simple
                                 ) {
                if (!simple) {
                    return;
                }
                if (auto named = std::get_if<PatternNamedBlockAst>(&*simple)) {
                    if (named->data) {
                        inspectExpr(*named->data);
                    }
                } else if (auto dynamic = std::get_if<PatternDynamicBlockAst>(&*simple)) {
                    inspectExpr(dynamic->source);
                    if (dynamic->data) {
                        inspectExpr(*dynamic->data);
                    }
                }
            };
            inspectSimple(packed.block);
            inspectSimple(packed.extraBlock);
        }
    }

    if (!needsState) {
        return nullptr;
    }

    auto state = std::make_shared<ExprState>();
    state->bindVars();
    state->bindFns();
    return state;
}

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

static ll::Expected<std::unique_ptr<CompiledExpr>>
compileExpr(PatternExpr const& expr, ExprState& state) {
    if (auto literal = std::get_if<PatternLiteralExpr>(&expr)) {
        return std::make_unique<CompiledExpr>(literal->value);
    }
    auto compiled = std::make_unique<CompiledExpr>();
    auto result   = compiled->compile(std::get<PatternRuntimeExpr>(expr).source, state);
    if (!result) {
        return ll::forwardError(result.error());
    }
    return compiled;
}

static std::optional<double> getLiteralExprValue(PatternExpr const& expr) {
    if (auto literal = std::get_if<PatternLiteralExpr>(&expr)) {
        return literal->value;
    }
    return std::nullopt;
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

static ll::Expected<PatternCachedSimpleBlock>
makeCachedLegacyIdBlock(double legacyIdValue, std::optional<double> dataValue) {
    auto legacyId = static_cast<uint>(std::llround(legacyIdValue));
    if (legacyIdValue < 0) {
        legacyId = 0;
    }

    ushort data = 0;
    if (dataValue) {
        auto rounded = std::llround(*dataValue);
        data         = static_cast<ushort>(rounded < 0 ? 0 : rounded);
    }

    return makeCachedSimpleBlock(
        Block::tryGetFromRegistry(legacyId, data),
        fmt::format("{}:{}", legacyId, data)
    );
}

static ll::Expected<PatternCompiledSimpleBlock>
compileSimpleBlock(PatternSimpleBlockSpec const& spec, ExprState& state) {
    if (auto named = std::get_if<PatternNamedBlockAst>(&spec)) {
        if (!named->data) {
            if (!named->states.empty()) {
                std::vector<std::pair<std::string, Block::BlockStateValueType>>
                    blockStates;
                blockStates.reserve(named->states.size());
                for (auto const& blockState : named->states) {
                    blockStates.push_back(compileState(blockState));
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
            auto data = compileExpr(*named->data, state);
            if (!data) {
                return ll::forwardError(data.error());
            }
            compiled.data = std::move(*data);
        }
        return compiled;
    }
    if (auto expr = std::get_if<PatternDynamicBlockAst>(&spec)) {
        auto literalSource = getLiteralExprValue(expr->source);
        auto literalData   = expr->data ? getLiteralExprValue(*expr->data) : std::nullopt;
        if (literalSource && (!expr->data || literalData)) {
            return makeCachedLegacyIdBlock(*literalSource, literalData);
        }

        PatternDynamicSimpleBlock compiled;
        auto                      blockExpr = compileExpr(expr->source, state);
        if (!blockExpr) {
            return ll::forwardError(blockExpr.error());
        }
        compiled.blockExpr = std::move(*blockExpr);
        if (expr->data) {
            auto data = compileExpr(*expr->data, state);
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
resolveSimpleBlock(PatternCompiledSimpleBlock const& block) {
    if (auto cached = std::get_if<PatternCachedSimpleBlock>(&block)) {
        return cached->block;
    }

    auto const& dynamic = std::get<PatternDynamicSimpleBlock>(block);

    std::optional<ushort> dataValue;
    if (dynamic.data) {
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
    double legacyId{};
    if (!dynamic.blockExpr->evalString(dynamicName, legacyId)) {
        return Block::tryGetFromRegistry(
            static_cast<uint>(legacyId < 0 ? 0 : legacyId),
            dataValue.value_or(0)
        );
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
        operation.block = resolveSimpleBlock(*entry.block);
    }
    if (entry.extraBlock) {
        operation.extraBlock = resolveSimpleBlock(*entry.extraBlock);
    }
    if (entry.cachedBlockEntityTag) {
        operation.blockActor = BlockActor::create(*entry.cachedBlockEntityTag, pos);
    }
    return operation;
}

BlockListPattern::BlockListPattern(
    std::vector<PatternCompiledEntry> entries,
    std::shared_ptr<ExprState>        exprState
)
: entries(std::move(entries)),
  exprState(std::move(exprState)) {}

ll::Expected<std::shared_ptr<BlockListPattern>>
BlockListPattern::create(BlockListPatternAst const& ast) {
    auto sharedExprState = makeExprState(ast);

    std::vector<PatternCompiledEntry> compiledEntries;
    compiledEntries.reserve(ast.entries.size());
    for (auto const& entry : ast.entries) {
        PatternCompiledEntry compiled;

        auto weight = compileExpr(entry.weight, *sharedExprState);
        if (!weight) {
            return ll::forwardError(weight.error());
        }
        compiled.weight = std::move(*weight);

        if (auto simple = std::get_if<PatternSimpleBlockSpec>(&entry.block.source)) {
            auto compiledBlock = compileSimpleBlock(*simple, *sharedExprState);
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
                auto compiledBlock = compileSimpleBlock(*packed.block, *sharedExprState);
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
                auto compiledExtra =
                    compileSimpleBlock(*packed.extraBlock, *sharedExprState);
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
    return std::make_shared<BlockListPattern>(
        std::move(compiledEntries),
        std::move(sharedExprState)
    );
}

ll::Expected<> BlockListPattern::prepare(CommandContextRef const& ctx) {
    if (!exprState) {
        return {};
    }
    auto dim = checkDimension(ctx);
    if (!dim) {
        return ll::makeStringError("failed to get dimension for pattern context");
    }
    exprState->setBlockSource(&dim->getBlockSourceFromMainChunkSource());
    auto region = checkRegion(ctx);
    if (region) {
        exprState->setBoundingBox(region->getBoundingBox());
    }
    return {};
}

BlockOperation BlockListPattern::pickBlock(BlockPos const& pos) const {
    if (entries.empty()) {
        return {};
    }

    if (exprState) {
        exprState->update(pos);
    }

    std::vector<double> weights;
    weights.reserve(entries.size());
    double total = 0.0;
    for (auto const& entry : entries) {
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
