#pragma once
#include "exprtk.hpp"

namespace we {
using Expression  = exprtk::expression<double>;
using SymbolTable = exprtk::symbol_table<double>;
using Parser      = exprtk::parser<double>;

template <class F>
class Function : public exprtk::igeneric_function<double> {
    F fn;

public:
    static constexpr bool return_string =
        !std::is_same_v<std::invoke_result_t<F, size_t, parameter_list_t>, double>;

    Function(F&& func, std::string const& param_seq = "")
    : exprtk::igeneric_function<double>(
          param_seq,
          return_string ? e_rtrn_string : e_rtrn_scalar
      ),
      fn(std::forward<F>(func)) {}

    type operator()(parameter_list_t params) override {
        return this->operator()(size_t(0), params);
    }
    type operator()(size_t const& idx, parameter_list_t params) override {
        if constexpr (return_string) {
            return type{};
        } else {
            return fn(idx, params);
        }
    }
    type operator()(std::string& out, parameter_list_t params) override {
        return this->operator()(size_t(0), out, params);
    }
    type
    operator()(size_t const& idx, std::string& out, parameter_list_t params) override {
        if constexpr (return_string) {
            out = fn(idx, params);
        }
        return type{};
    }
};
} // namespace we