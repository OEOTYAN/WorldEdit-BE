//
// Created by OEOTYAN on 2023/02/03.
//

#include "Global.h"
#include "FlatBrush.h"
#include "mc/Level.hpp"
#include "WorldEdit.h"
#include "mc/Dimension.hpp"
#include "utils/ColorTool.h"
#include "region/CuboidRegion.h"

namespace worldedit {
    FlatBrush::FlatBrush(unsigned short size, float density) : Brush(size, nullptr), density(density) {}

    long long FlatBrush::set(Player* player, BlockInstance blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return -2;
        }
        auto pos = blockInstance.getPosition();

        auto xuid = player->getXuid();
        auto& playerData = getPlayersData(xuid);
        auto dimID = player->getDimensionId();
        auto blockSource = &player->getRegion();

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
                    heightMap[cx * sizez + cz] =
                        std::lerp(topy + 0.5, pos.y + 0.5,
                                  static_cast<double>(smoothBrushAlpha(
                                      static_cast<float>(sqrt(pow2(x - pos.x) + pow2(z - pos.z))), density, 1, size)));
                    box.merge(BlockPos(x, topy, z));
                }
            }
        box.min.y = std::max(static_cast<int>(range.min), box.min.y - 1);
        box.max.y = std::min(static_cast<int>(range.max), box.max.y + 1);
        
        if (playerData.maxHistoryLength > 0) {
            auto history = playerData.getNextHistory();
            *history = Clipboard(box.max - box.min);
            history->playerRelPos.x = dimID;
            history->playerPos = box.min;
            box.forEachBlockInBox([&](const BlockPos& pos) {
                auto localPos = pos - box.min;
                auto blockInstance = blockSource->getBlockInstance(pos);
                history->storeBlock(blockInstance, localPos);
            });
        }

        // logger().debug("applyHeightMap");
        CuboidRegion(box, dimID).applyHeightMap(heightMap, xuid, mask);

        return -2;
    }

}  // namespace worldedit