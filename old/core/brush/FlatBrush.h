//
// Created by OEOTYAN on 2023/02/03.
//

#pragma once

#include "Brush.h"

namespace worldedit {
    class FlatBrush final : public Brush {
       public:
        float density;
        FlatBrush(unsigned short size, float density);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit