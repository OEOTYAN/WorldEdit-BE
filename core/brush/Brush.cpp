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
    }
    long long Brush::set(Player* player, ::BlockInstance blockInstance) {
        return -1;
    }
    Brush::Brush(unsigned short s, BlockPattern* bp) : size(s), pattern(bp) {
        setted = true;
    }
}  // namespace worldedit