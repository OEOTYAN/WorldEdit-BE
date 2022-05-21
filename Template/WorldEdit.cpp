//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#include "WorldEdit.h"
#include "HookAPI.h"
#include <MC/Block.hpp>
#include <MC/BlockLegacy.hpp>
#include <fstream>

// unsigned int const &  Block*
THook(void,
      "?setRuntimeId@Block@@IEBAXAEBI@Z",
      Block* block,
      unsigned int const& id) {
    auto& blockName = worldedit::getBlockNameMap();
    auto blockid = block->getId();
    blockName[blockid] = block->getTypeName();
    std::ofstream ofs;
    ofs.open("text.txt", std::ios::out | std::ios::app);
    ofs << id << " " << blockid << " " << block->getTypeName() << std::endl;
    ofs.close();
    return original(block, id);
}

namespace worldedit {
    WE& getMod() {
        static WE t;
        return t;
    }

    std::unordered_map<int, std::string>& getBlockNameMap() {
        static std::unordered_map<int, std::string> blockName;
        return blockName;
    }
    std::string getBlockName(int id) {
        auto& blockName = worldedit::getBlockNameMap();
        if (blockName.find(id) != blockName.end())
            return blockName[id];
        return "minecraft:air";
    }
    void WE::renderMVpos() {
        for (auto& PosPair : playerMainPosMap) {
            if (PosPair.second.second.first <= 0) {
                PosPair.second.second.first = 40;
                worldedit::spawnCuboidParticle(
                    AABB(
                        Vec3(PosPair.second.first) - Vec3(0.07f, 0.07f, 0.07f),
                        Vec3(PosPair.second.first) + Vec3(1.07f, 1.07f, 1.07f)),
                    GRAPHIC_COLOR::RED, PosPair.second.second.second);
            }
            PosPair.second.second.first--;
        }
        for (auto& PosPair : playerVicePosMap) {
            if (PosPair.second.second.first <= 0) {
                PosPair.second.second.first = 40;
                worldedit::spawnCuboidParticle(
                    AABB(
                        Vec3(PosPair.second.first) - Vec3(0.06f, 0.06f, 0.06f),
                        Vec3(PosPair.second.first) + Vec3(1.06f, 1.06f, 1.06f)),
                    GRAPHIC_COLOR::BLUE, PosPair.second.second.second);
            }
            PosPair.second.second.first--;
        }
    }
}  // namespace worldedit