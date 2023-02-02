//
// Created by OEOTYAN on 2023/02/01.
//

#pragma once

#include "ColorBrush.h"
#include "WorldEdit.h"
#include "utils/ColorTool.h"
#include "mc/StaticVanillaBlocks.hpp"
#include "mc/BlockSource.hpp"
#include "mc/BlockInstance.hpp"
#include "store/Patterns.h"

namespace worldedit {
    ColorBrush::ColorBrush(unsigned short size,
                           float density,
                           float opacity,
                           mce::Color color,
                           std::unique_ptr<Pattern> bp,
                           bool mixbox)
        : Brush(size, std::move(bp)), density(density), opacity(opacity), color(color), useMixboxLerp(mixbox) {}

    long long ColorBrush::set(Player* player, BlockInstance blockInstance) {
        long long iter = 0;
        if (blockInstance == BlockInstance::Null) {
            return -2;
        }
        std::queue<BlockPos> q;
        auto pos0 = blockInstance.getPosition();
        q.push(pos0);

        auto xuid = player->getXuid();
        auto& playerData = getPlayersData(xuid);
        auto blockSource = blockInstance.getBlockSource();

        EvalFunctions f;
        f.setbs(blockSource);
        std::unordered_map<std::string, double> variables;
        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        variables["px"] = playerPos.x;
        variables["py"] = playerPos.y;
        variables["pz"] = playerPos.z;
        variables["pp"] = playerRot.x;
        variables["pt"] = playerRot.y;
        std::unordered_set<BlockPos> s;
        s.insert(pos0);

        BoundingBox boundingBox = BoundingBox(pos0, pos0);

        mce::Color finalColor = color;

        auto& cmap = getBlockColorMap();

        if (pattern != nullptr) {
            f.setPos(pos0);
            variables["rx"] = pos0.x;
            variables["ry"] = pos0.y;
            variables["rz"] = pos0.z;
            variables["x"] = 0;
            variables["y"] = 0;
            variables["z"] = 0;
            variables["cx"] = 0;
            variables["cy"] = 0;
            variables["cz"] = 0;
            variables["ox"] = pos0.x - playerPos.x;
            variables["oy"] = pos0.y - playerPos.y;
            variables["oz"] = pos0.z - playerPos.z;
            Block* blockp = pattern->getBlock(variables, f);
            if (cmap.find(blockp) == cmap.end()) {
                return -2;
            }
            finalColor = cmap[blockp];
        }

        while (!q.empty()) {
            auto pos1 = q.front();
            q.pop();
            boundingBox.merge(pos1);
            BoundingBox(pos1 - 1, pos1 + 1).forEachBlockInBox([&](const BlockPos& tmpPos) {
                if (&blockSource->getBlock(tmpPos) != BedrockBlocks::mAir && tmpPos != pos1 &&
                    pos0.distanceTo(tmpPos) <= 0.5 + size && s.find(tmpPos) == s.end()) {
                    int counts = 0;
                    for (auto& calPos : tmpPos.getNeighbors()) {
                        counts += !blockSource->getBlock(calPos).isSolid();
                    }
                    if (counts > 0) {
                        q.push(tmpPos);
                        s.insert(tmpPos);
                    }
                }
            });
        }

        worldedit::Clipboard* history = nullptr;

        if (playerData.maxHistoryLength > 0) {
            history = playerData.getNextHistory();
            *history = Clipboard(boundingBox.max - boundingBox.min);
            history->playerRelPos.x = blockInstance.getDimensionId();
            history->playerPos = boundingBox.min;
            for (auto& pos1 : s) {
                auto localPos = pos1 - boundingBox.min;
                auto bi = blockSource->getBlockInstance(pos1);
                history->storeBlock(bi, localPos);
            }
        }

        for (auto& pos1 : s) {
            f.setPos(pos1);
            variables["x"] = static_cast<double>(pos1.x - pos0.x) / size;
            variables["y"] = static_cast<double>(pos1.y - pos0.y) / size;
            variables["z"] = static_cast<double>(pos1.z - pos0.z) / size;
            variables["rx"] = pos1.x;
            variables["ry"] = pos1.y;
            variables["rz"] = pos1.z;
            variables["cx"] = pos1.x - pos0.x;
            variables["cy"] = pos1.y - pos0.y;
            variables["cz"] = pos1.z - pos0.z;
            variables["ox"] = pos1.x - playerPos.x;
            variables["oy"] = pos1.y - playerPos.y;
            variables["oz"] = pos1.z - playerPos.z;
            // logger().debug("maskFunc");
            maskFunc(f, variables, [&]() mutable {
                auto* block = const_cast<Block*>(&blockSource->getBlock(pos1));
                mce::Color hereColor(0, 0, 0, 1);
                if (cmap.find(block) != cmap.end()) {
                    hereColor = cmap[block];
                }

                hereColor = useMixboxLerp ? mixboxLerp(hereColor, finalColor,
                                                       smoothBrushAlpha(static_cast<float>(pos1.distanceTo(pos0)),
                                                                        density, opacity, size))
                                          : linearLerp(hereColor, finalColor,
                                                       smoothBrushAlpha(static_cast<float>(pos1.distanceTo(pos0)),
                                                                        density, opacity, size));

                double minDist = DBL_MAX;
                Block* minBlock = nullptr;
                for (auto& i : getColorBlockMap()) {
                    if (i.first.a == 1) {
                        auto dst = i.first.distanceTo(hereColor);
                        if (dst < minDist) {
                            minDist = dst;
                            minBlock = i.second;
                        }
                    }
                }

                iter += playerData.setBlockSimple(blockSource, f, variables, pos1, minBlock);
            });
        }
        return iter;
    }
}  // namespace worldedit