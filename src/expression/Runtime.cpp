#include "expression/Runtime.h"

namespace we {
static exprtk::igeneric_function<double>::return_type toReturnType(ExprReturnKind kind) {
    using Base = exprtk::igeneric_function<double>;
    switch (kind) {
    case ExprReturnKind::String:
        return Base::e_rtrn_string;
    case ExprReturnKind::Overload:
        return Base::e_rtrn_overload;
    case ExprReturnKind::Scalar:
    default:
        return Base::e_rtrn_scalar;
    }
}

ExprFnBinding::ExprFnBinding(ExprCtx& ctx, ExprFnDesc const& desc)
: exprtk::igeneric_function<double>(desc.signature, toReturnType(desc.returnKind)),
  mCtx(&ctx),
  mFn(desc.fn) {}

ExprFnBinding::type ExprFnBinding::operator()(parameter_list_t params) {
    return this->operator()(size_t(0), params);
}

ExprFnBinding::type ExprFnBinding::operator()(size_t const&, parameter_list_t params) {
    mLast = mFn(*mCtx, params);
    return mLast.scalar();
}

ExprFnBinding::type ExprFnBinding::operator()(std::string& out, parameter_list_t params) {
    return this->operator()(size_t(0), out, params);
}

ExprFnBinding::type ExprFnBinding::operator()(
    size_t const&, std::string& out, parameter_list_t params
) {
    mLast = mFn(*mCtx, params);
    if (auto ptr = mLast.string()) {
        out = *ptr;
    } else {
        out.clear();
    }
    return mLast.scalar();
}

void ExprState::bindVars() {
    table.add_variable("rx", realPos.x);
    table.add_variable("ry", realPos.y);
    table.add_variable("rz", realPos.z);
    table.add_variable("x", unitPos.x);
    table.add_variable("y", unitPos.y);
    table.add_variable("z", unitPos.z);
    table.add_constants();
}

void ExprState::bindFns() {
    for (auto const& desc : ExprRegistry::instance().all()) {
        auto fn = std::make_unique<ExprFnBinding>(*this, desc);
        table.add_function(desc.name, *fn);
        functions.push_back(std::move(fn));
    }
}

CompiledExpr::CompiledExpr(double literal) : mLiteral(literal) {}

ll::Expected<> CompiledExpr::compile(std::string const& source, ExprState& state) {
    mLiteralMode = false;
    mExpr.register_symbol_table(state.table);

    ExprParser parser;
    if (!parser.compile(source, mExpr)) {
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

double CompiledExpr::evalScalar() const {
    if (mLiteralMode) {
        return mLiteral;
    }
    return mExpr.value();
}

bool CompiledExpr::evalString(std::string_view& out) const {
    if (mLiteralMode) {
        return false;
    }
    mExpr.value();
    auto const& results = mExpr.results();
    if (results.count() != 1) {
        return false;
    }
    if (results[0].type != exprtk::type_store<double>::e_string) {
        return false;
    }
    auto str = exprtk::type_store<double>::string_view(results[0]);
    out      = std::string_view(str.begin(), str.size());
    return true;
}
} // namespace we
