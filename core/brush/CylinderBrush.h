//
// Created by OEOTYAN on 2022/06/10.
//
#pragma once
#ifndef WORLDEDIT_CYLINDERBRUSH_H
#define WORLDEDIT_CYLINDERBRUSH_H

#include "Brush.h"
namespace worldedit {
    class CylinderBrush : public Brush {
       public:
        bool hollow = false;
        int  height = 0;
        CylinderBrush(unsigned short, BlockPattern*, int, bool);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_CYLINDERBRUSH_H