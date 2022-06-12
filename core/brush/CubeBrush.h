//
// Created by OEOTYAN on 2022/06/11.
//
#pragma once
#ifndef WORLDEDIT_CUBEBRUSH_H
#define WORLDEDIT_CUBEBRUSH_H

#include "Brush.h"
namespace worldedit {
    class CubeBrush : public Brush {
       public:
        bool hollow = false;
        CubeBrush(unsigned short, BlockPattern*, bool);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_CUBEBRUSH_H