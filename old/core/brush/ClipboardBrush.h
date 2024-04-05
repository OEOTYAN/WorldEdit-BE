//
// Created by OEOTYAN on 2022/06/10.
//
#pragma once

#include "Brush.h"
#include "store/Clipboard.hpp"
namespace worldedit {
    class ClipboardBrush final : public Brush {
       public:
        Clipboard clipboard;
        bool center = true;
        bool ignoreAir = false;
        ClipboardBrush(unsigned short, Clipboard&, bool, bool);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit
