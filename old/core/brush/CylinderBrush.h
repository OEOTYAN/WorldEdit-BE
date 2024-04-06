#pragma once

#include "Brush.h"
namespace we {
class CylinderBrush : public Brush {
public:
    bool hollow = false;
    int  height = 0;
    CylinderBrush(unsigned short, std::unique_ptr<Pattern>, int, bool);
    long long set(Player* player, BlockInstance blockInstance) override;
};
} // namespace we
