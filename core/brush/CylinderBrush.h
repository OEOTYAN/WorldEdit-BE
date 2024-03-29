//
// Created by OEOTYAN on 2022/06/10.
//
#pragma once

#include "Brush.h"
namespace worldedit {
    class CylinderBrush final : public Brush {
       public:
        bool hollow = false;
        int height = 0;
        CylinderBrush(unsigned short, std::unique_ptr<Pattern>, int, bool);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit
