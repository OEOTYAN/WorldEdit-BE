//
// Created by OEOTYAN on 2022/06/10.
//
#pragma once

#include "Brush.h"
#include "image/Image.h"
namespace worldedit {
    class ImageHeightmapBrush final : public Brush {
       public:
        int height = 0;
        bool rotation;
        Texture2D texture;
        ImageHeightmapBrush(unsigned short, int, Texture2D const&, bool);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit
