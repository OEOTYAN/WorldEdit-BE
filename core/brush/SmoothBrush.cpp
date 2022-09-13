//
// Created by OEOTYAN on 2022/06/10.
//

#include "SmoothBrush.h"
#include "store/BlockPattern.hpp"
#include "region/CuboidRegion.h"
#include "eval/blur.hpp"

namespace worldedit {
    SmoothBrush::SmoothBrush(unsigned short s, int a) : Brush(s, nullptr), ksize(a) {}
    long long SmoothBrush::set(Player* player, BlockInstance blockInstance) {
        auto pos = blockInstance.getPosition();
        auto& mod = worldedit::getMod();
        auto xuid = player->getXuid();

        BoundingBox box(pos - size, pos + size + BlockPos(0, 10, 0));

        auto dimID = player->getDimensionId();
        auto blockSource = &player->getRegion();
        auto region = CuboidRegion(box, dimID);
        if (mod.maxHistoryLength > 0) {
            auto history = mod.getPlayerNextHistory(xuid);
            *history = Clipboard(box.max - box.min);
            history->playerRelPos.x = dimID;
            history->playerPos = box.min;
            box.forEachBlockInBox([&](const BlockPos& pos) {
                auto localPos = pos - box.min;
                auto blockInstance = blockSource->getBlockInstance(pos);
                history->storeBlock(blockInstance, localPos);
            });
        }

        auto heightMap = region.getHeightMap(mask);
        int sizex = box.max.x - box.min.x + 1;
        int sizez = box.max.z - box.min.z + 1;

        auto smoothedHeightMap = blur2D(heightMap, ksize, sizex, sizez);
        region.applyHeightMap(smoothedHeightMap, mask);

        return (size * 2 + 1) * (size * 2 + 1) * (size * 2 + 11);
    }

}  // namespace worldedit