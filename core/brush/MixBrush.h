//
// Created by OEOTYAN on 2023/02/01.
//

#pragma once

#include "Brush.h"
namespace worldedit {
    class MixBrush final : public Brush {
       public:
        float density = 1;
        float opacity = 1;
        bool useMixboxLerp = true;
        MixBrush(unsigned short size, float density, float opacity,bool mixbox);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit
