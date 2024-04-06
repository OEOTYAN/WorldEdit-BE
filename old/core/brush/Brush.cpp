#include "Brush.h"
#include "store/Patterns.h"
#include <mc/BlockInstance.hpp>
#include <mc/Player.hpp>
namespace we {
long long Brush::set(Player* player, ::BlockInstance blockInstance) { return -2; }
long long Brush::lset(Player* player, ::BlockInstance blockInstance) { return -2; }
bool      Brush::maskFunc(
    EvalFunctions&                                   func,
    const phmap::flat_hash_map<std::string, double>& var,
    std::function<void()> const&                     todo
) {
    if (mask != "") {
        if (cpp_eval::eval<double>(mask, var, func) > 0.5) {
            todo();
            return true;
        }
    } else {
        todo();
        return true;
    }
    return false;
}

Brush::Brush(unsigned short s, std::unique_ptr<Pattern> p)
: size(s),
  pattern(std::move(p)) {}
} // namespace we