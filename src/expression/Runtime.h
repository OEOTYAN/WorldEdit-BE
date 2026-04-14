#pragma once

#include "expression/Registry.h"

#include <ll/api/Expected.h>
#include <ll/api/utils/ErrorUtils.h>

namespace we {
class ExprFnBinding : public exprtk::igeneric_function<double> {
public:
    using Base        = exprtk::igeneric_function<double>;
    using GenericType = Base::generic_type;
    using ScalarView  = GenericType::scalar_view;
    using VectorView  = GenericType::vector_view;
    using StringView  = GenericType::string_view;

    ExprFnBinding(ExprCtx& ctx, ExprFnDesc const& desc);

    type operator()(parameter_list_t params) override;
    type operator()(size_t const& overloadIndex, parameter_list_t params) override;
    type operator()(std::string& out, parameter_list_t params) override;
    type operator()(
        size_t const&    overloadIndex,
        std::string&     out,
        parameter_list_t params
    ) override;

private:
    ExprCtx*          mCtx;
    ExprFn            mFn;
    mutable ExprValue mLast;
};

struct ExprState : ExprCtx {
    ExprTable                                   table;
    std::vector<std::unique_ptr<ExprFnBinding>> functions;

    void bindVars();
    void bindFns();
};

class CompiledExpr {
public:
    CompiledExpr() = default;
    explicit CompiledExpr(double literal);

    ll::Expected<> compile(std::string const& source, ExprState& state);

    [[nodiscard]] double evalScalar() const;
    bool                 evalString(std::string_view& out) const;

private:
    bool   mLiteralMode = true;
    double mLiteral     = 0.0;
    Expr   mExpr;
};
} // namespace we
