//
// Created by OEOTYAN on 2022/06/10.
//
#include "Brush.h"
#include <mc/BlockInstance.hpp>
#include <mc/Player.hpp>
#include "store/Patterns.h"
namespace worldedit {
    Brush::~Brush() {
    }
    long long Brush::set(Player* player, ::BlockInstance blockInstance) {
        return -2;
    }
    long long Brush::lset(Player* player, ::BlockInstance blockInstance) {
        return -2;
    }
    bool Brush::maskFunc(EvalFunctions& func,
                         const std::unordered_map<std::string, double>& var,
                         std::function<void()> const& todo) {
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

    Brush::Brush(unsigned short s, std::unique_ptr<Pattern> p) : size(s), pattern(std::move(p)) {}
}  // namespace worldedit