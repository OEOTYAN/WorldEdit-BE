
#pragma once

#include "Brush.h"
#include "Globals.h"

namespace we {
class ColorBrush : public Brush {
public:
    float      density       = 1;
    float      opacity       = 1;
    bool       useMixboxLerp = true;
    mce::Color color         = {1, 1, 1, 1};
    ColorBrush(
        unsigned short           size,
        float                    density,
        float                    opacity,
        mce::Color               color,
        std::unique_ptr<Pattern> bp,
        bool                     mixbox
    );
    long long set(Player* player, BlockInstance blockInstance) override;
};
} // namespace we
