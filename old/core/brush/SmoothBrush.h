#pragma once

#include "Brush.h"
namespace we {
class SmoothBrush : public Brush {
public:
    float density = 1;
    int   ksize   = 0;
    SmoothBrush(unsigned short, int, float);
    long long set(Player* player, BlockInstance blockInstance) override;
};
} // namespace we
