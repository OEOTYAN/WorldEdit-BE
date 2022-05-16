//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#include "WorldEdit.h"

namespace worldedit {
    WE& getMod() {
        static WE t;
        return t;
    }
    void WE::renderMVpos() {
        for (auto& PosPair : playerMainPosMap) {
            if (PosPair.second.second.first <= 0) {
                PosPair.second.second.first = 40;
                worldedit::spawnCuboidParticle(
                    AABB(Vec3(PosPair.second.first) -
                             Vec3(0.07f, 0.07f, 0.07f),
                         Vec3(PosPair.second.first) +
                             Vec3(1.07f, 1.07f, 1.07f)),
                    GRAPHIC_COLOR::RED, PosPair.second.second.second);
            }
            PosPair.second.second.first--;
        }
        for (auto& PosPair : playerVicePosMap) {
            if (PosPair.second.second.first <= 0) {
                PosPair.second.second.first = 40;
                worldedit::spawnCuboidParticle(
                    AABB(Vec3(PosPair.second.first) -
                             Vec3(0.07f, 0.07f, 0.07f),
                         Vec3(PosPair.second.first) +
                             Vec3(1.07f, 1.07f, 1.07f)),
                    GRAPHIC_COLOR::BLUE, PosPair.second.second.second);
            }
            PosPair.second.second.first--;
        }
    }
}  // namespace worldedit