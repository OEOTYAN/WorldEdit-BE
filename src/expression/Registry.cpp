#include "expression/Registry.h"

#include <algorithm>

namespace we {
double ExprValue::scalar(double fallback) const {
    if (auto ptr = std::get_if<double>(&data)) {
        return *ptr;
    }
    return fallback;
}

std::string const* ExprValue::string() const { return std::get_if<std::string>(&data); }

ExprRegistry& ExprRegistry::instance() {
    static ExprRegistry registry;
    return registry;
}

bool ExprRegistry::add(ExprFnDesc desc) {
    auto duplicate =
        std::find_if(mEntries.begin(), mEntries.end(), [&](ExprFnDesc const& item) {
            return item.name == desc.name;
        });
    if (duplicate != mEntries.end()) {
        return false;
    }
    mEntries.emplace_back(std::move(desc));
    return true;
}

ExprRegister::ExprRegister(
    char const*    name,
    char const*    signature,
    ExprReturnKind returnKind
)
: mDesc{std::string{name}, std::string{signature}, returnKind, {}} {}

bool ExprRegister::operator|(ExprFn fn) const noexcept {
    auto desc = mDesc;
    desc.fn   = std::move(fn);
    return ExprRegistry::instance().add(std::move(desc));
}
} // namespace we
