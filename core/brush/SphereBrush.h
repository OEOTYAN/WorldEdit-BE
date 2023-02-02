//
// Created by OEOTYAN on 2022/06/10.
//
#pragma once

#include "Brush.h"
namespace worldedit {
    class SphereBrush : public Brush {
       public:
        bool hollow = false;
        SphereBrush(unsigned short, std::unique_ptr<Pattern>, bool);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit
