
#pragma once

#include "Brush.h"

namespace we {
class FlatBrush : public Brush {
public:
    float density;
    FlatBrush(unsigned short size, float density);
    long long set(Player* player, BlockInstance blockInstance) override;
};
} // namespace we