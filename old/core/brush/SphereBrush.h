#pragma once

#include "Brush.h"
namespace we {
class SphereBrush : public Brush {
public:
    bool hollow = false;
    SphereBrush(unsigned short, std::unique_ptr<Pattern>, bool);
    long long set(Player* player, BlockInstance blockInstance) override;
};
} // namespace we
