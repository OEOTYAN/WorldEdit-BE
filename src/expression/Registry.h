#pragma once

#include "exprtk.hpp"
#include "expression/Context.h"

#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace we {
using Expr       = exprtk::expression<double>;
using ExprTable  = exprtk::symbol_table<double>;
using ExprParser = exprtk::parser<double>;
using ExprArgs   = exprtk::igeneric_function<double>::parameter_list_t;

struct ExprValue {
    using Data = std::variant<double, std::string>;

    Data data = 0.0;

    ExprValue() = default;
    ExprValue(double value) : data(value) {}
    ExprValue(std::string value) : data(std::move(value)) {}

    [[nodiscard]] double scalar(double fallback = 0.0) const;
    [[nodiscard]] std::string const* string() const;
};

using ExprFn = std::function<ExprValue(ExprCtx&, ExprArgs)>;

enum class ExprReturnKind {
    Scalar,
    String,
    Overload,
};

struct ExprFnDesc {
    std::string    name;
    std::string    signature;
    ExprReturnKind returnKind = ExprReturnKind::Scalar;
    ExprFn         fn;
};

class ExprRegistry {
public:
    static ExprRegistry& instance();

    bool add(ExprFnDesc desc);

    [[nodiscard]] std::vector<ExprFnDesc> const& all() const { return mEntries; }

private:
    std::vector<ExprFnDesc> mEntries;
};

class ExprRegister {
    ExprFnDesc mDesc;

public:
    ExprRegister(char const* name, char const* signature, ExprReturnKind returnKind);

    bool operator|(ExprFn fn) const noexcept;
};

#define WE_EXPR_SCALAR(name, signature)                                                 \
    [[maybe_unused]] static bool g_expr_##name =                                        \
        ::we::ExprRegister{#name, signature, ::we::ExprReturnKind::Scalar} |

#define WE_EXPR_STRING(name, signature)                                                 \
    [[maybe_unused]] static bool g_expr_##name =                                        \
        ::we::ExprRegister{#name, signature, ::we::ExprReturnKind::String} |

#define WE_EXPR_OVERLOAD(name, signature)                                               \
    [[maybe_unused]] static bool g_expr_##name =                                        \
        ::we::ExprRegister{#name, signature, ::we::ExprReturnKind::Overload} |
} // namespace we
