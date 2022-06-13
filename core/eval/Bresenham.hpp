//
// Created by OEOTYAN on 2022/06/14.
//
#pragma once
#ifndef WORLDEDIT_BRESENHAM_H
#define WORLDEDIT_BRESENHAM_H
#include "pch.h"
namespace worldedit {
    void plotLine(BlockPos const& pos0, BlockPos const& pos1, std::function<void(class BlockPos const&)> const& todo);
}

#endif  // WORLDEDIT_BRESENHAM_H