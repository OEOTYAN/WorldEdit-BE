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

static optional_ref<Block const>
resolveBlock(PatternCompiledEntry const& entry, BlockPos const& pos) {
    std::string_view blockName;
    if (entry.blockName) {
        blockName = *entry.blockName;
    } else {
        entry.blockExpr->updateVariables(pos);
        if (!entry.blockExpr->evalString(blockName)) {
            return nullptr;
        }
    }
    if (entry.data) {
        entry.data->updateVariables(pos);
        auto dataEval  = std::llround(entry.data->evalScalar());
        auto dataValue = static_cast<ushort>(dataEval < 0 ? 0 : dataEval);
        return Block::tryGetFromRegistry(blockName, dataValue);
    }
    return Block::tryGetFromRegistry(blockName);
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

        if (auto blockName = std::get_if<std::string>(&entry.block.source)) {
            compiled.blockName = *blockName;
        } else {
            auto blockExpr =
                compileExpr(std::get<PatternRuntimeExpr>(entry.block.source));
            if (!blockExpr) {
                return ll::forwardError(blockExpr.error());
            }
            compiled.blockExpr = std::move(*blockExpr);
        }

        if (entry.data) {
            auto data = compileExpr(*entry.data);
            if (!data) {
                return ll::forwardError(data.error());
            }
            compiled.data = std::move(*data);
        }

        compiledEntries.push_back(std::move(compiled));
    }
    return std::make_shared<BlockListPattern>(std::move(compiledEntries));
}

optional_ref<Block const> BlockListPattern::pickBlock(BlockPos const& pos) const {
    if (entries.empty()) {
        return nullptr;
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
        return nullptr;
    }

    auto   target = ll::utils::random_utils::rand<double>(total);
    double sum    = 0.0;
    for (size_t i = 0; i < entries.size(); ++i) {
        sum += weights[i];
        if (target <= sum || i + 1 == entries.size()) {
            return resolveBlock(entries[i], pos);
        }
    }
    return nullptr;
}
} // namespace we
