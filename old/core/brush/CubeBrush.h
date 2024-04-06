#pragma once

#include "Brush.h"
namespace we {
class CubeBrush : public Brush {
public:
    bool hollow = false;
    CubeBrush(unsigned short, std::unique_ptr<Pattern>, bool);
    long long set(Player* player, BlockInstance blockInstance) override;
};
} // namespace we
