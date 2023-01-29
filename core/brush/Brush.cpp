//
// Created by OEOTYAN on 2022/06/10.
//
#include "Brush.h"
#include <MC/BlockInstance.hpp>
#include <MC/Player.hpp>
#include "store/BlockPattern.hpp"
namespace worldedit {
    Brush::~Brush() {
        delete pattern;
        pattern = nullptr;
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

    Brush::Brush(unsigned short s, BlockPattern* bp) : size(s), pattern(bp) {
        setted = true;
    }
}  // namespace worldedit