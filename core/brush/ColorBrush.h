//
// Created by OEOTYAN on 2023/02/01.
//

#pragma once

#include "Global.h"
#include "Brush.h"

namespace worldedit {
    class ColorBrush : public Brush {
       public:
        float density = 1;
        float opacity = 1;
        bool useMixboxLerp = true;
        mce::Color color = {1, 1, 1, 1};
        ColorBrush(unsigned short size, float density, float opacity, mce::Color color, std::unique_ptr<Pattern> bp,bool mixbox);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit
