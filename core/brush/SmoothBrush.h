//
// Created by OEOTYAN on 2022/06/10.
//
#pragma once
#ifndef WORLDEDIT_SMOOTHBRUSH_H
#define WORLDEDIT_SMOOTHBRUSH_H

#include "Brush.h"
namespace worldedit {
    class SmoothBrush : public Brush {
       public:
        int ksize = 0;
        SmoothBrush(unsigned short, int);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_SMOOTHBRUSH_H