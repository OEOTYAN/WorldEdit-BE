#pragma once

#include "Brush.h"
#include "store/Clipboard.hpp"
namespace we {
class ClipboardBrush : public Brush {
public:
    Clipboard clipboard;
    bool      center    = true;
    bool      ignoreAir = false;
    ClipboardBrush(unsigned short, Clipboard&, bool, bool);
    long long set(Player* player, BlockInstance blockInstance) override;
};
} // namespace we
