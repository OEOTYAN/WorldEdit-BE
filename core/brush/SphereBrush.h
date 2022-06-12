//
// Created by OEOTYAN on 2022/06/10.
//
#pragma once
#ifndef WORLDEDIT_SPHEREBRUSH_H
#define WORLDEDIT_SPHEREBRUSH_H

#include "Brush.h"
namespace worldedit {
    class SphereBrush : public Brush {
       public:
        bool hollow = false;
        SphereBrush(unsigned short, BlockPattern*, bool);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_SPHEREBRUSH_H