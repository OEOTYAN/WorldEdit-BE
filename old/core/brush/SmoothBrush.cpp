//
// Created by OEOTYAN on 2022/06/10.
//

#include "Globals.h"
#include "SmoothBrush.h"
#include "utils/ColorTool.h"
#include "store/Patterns.h"
#include "WorldEdit.h"
#include "mc/Dimension.hpp"
#include "region/CuboidRegion.h"
#include "eval/blur.hpp"

namespace worldedit {
    SmoothBrush::SmoothBrush(unsigned short s, int a, float d) : Brush(s, nullptr), ksize(a), density(d) {}
    long long SmoothBrush::set(Player* player, BlockInstance blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return -2;
        }
        auto pos = blockInstance.getPosition();

        auto xuid = player->getXuid();
        auto& playerData = getPlayersData(xuid);
        auto dimID = player->getDimensionId();
        auto blockSource = &player->getDimensionBlockSource();

        BoundingBox box(pos - size, pos + size);
        box.min.y = pos.y;
        box.max.y = pos.y;

        int sizex = box.max.x - box.min.x + 1;
        int sizez = box.max.z - box.min.z + 1;
        std::vector<double> heightMap(sizex * sizez, -1e200);
        auto range =
            reinterpret_cast<Dimension*>(Global<Level>->getDimension(dimID).mHandle.lock().get())->getHeightRange();
        for (int x = box.min.x; x <= box.max.x; ++x)
            for (int z = box.min.z; z <= box.max.z; ++z) {
                auto topy = getHighestTerrainBlock(blockSource, x, z, range.min, range.max, mask);
                if (topy >= range.min && topy <= range.max) {
                    int cx = x - box.min.x;
                    int cz = z - box.min.z;
                    heightMap[cx * sizez + cz] = topy + 0.5;
                    box.merge(BlockPos(x, topy, z));
                }
            }
        box.min.y = std::max(static_cast<int>(range.min), box.min.y - 1);
        box.max.y = std::min(static_cast<int>(range.max), box.max.y + 1);

        if (playerData.maxHistoryLength > 0) {
            auto& history = playerData.getNextHistory();
            history = std::move(Clipboard(box.max - box.min));
            history.playerRelPos.x = dimID;
            history.playerPos = box.min;
            box.forEachBlockInBox([&](const BlockPos& pos) {
                auto localPos = pos - box.min;
                auto blockInstance = blockSource->getBlockInstance(pos);
                history.storeBlock(blockInstance, localPos);
            });
        }

        auto smoothedHeightMap = blur2D(heightMap, ksize, sizex, sizez);

        for (int x = box.min.x; x <= box.max.x; ++x)
            for (int z = box.min.z; z <= box.max.z; ++z) {
                int cx = x - box.min.x;
                int cz = z - box.min.z;
                smoothedHeightMap[cx * sizez + cz] =
                    std::lerp(heightMap[cx * sizez + cz], smoothedHeightMap[cx * sizez + cz],
                              static_cast<double>(smoothBrushAlpha(
                                  static_cast<float>(sqrt(pow2(x - pos.x) + pow2(z - pos.z))), density, 1, size)));
            }

        CuboidRegion(box, dimID).applyHeightMap(smoothedHeightMap, xuid, mask);

        return -2;
    }

}  // namespace worldedit