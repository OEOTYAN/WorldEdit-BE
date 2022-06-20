//
// Created by OEOTYAN on 2022/06/10.
//
#pragma once
#ifndef WORLDEDIT_IMAGEHEIGHTMAPBRUSH_H
#define WORLDEDIT_IMAGEHEIGHTMAPBRUSH_H

#include "Brush.h"
#include "image/Image.h"
namespace worldedit {
    class ImageHeightmapBrush : public Brush {
        public:
        int height = 0;
        bool rotation;
        Texture2D texture;
        ImageHeightmapBrush(unsigned short, int,Texture2D const&,bool);
        long long set(Player* player, BlockInstance blockInstance) override;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_IMAGEHEIGHTMAPBRUSH_H