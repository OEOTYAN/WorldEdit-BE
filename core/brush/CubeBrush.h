//
// Created by OEOTYAN on 2022/06/11.
//
#pragma once

#include "Brush.h"
namespace worldedit {
    class CubeBrush final : public Brush {
       public:
        bool hollow = false;
        CubeBrush(unsigned short, std::unique_ptr<Pattern>, bool);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit
