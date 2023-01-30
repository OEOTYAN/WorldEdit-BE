//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once

#include "Global.h"
#include "data/PlayerData.h"

#define WE_DIR (std::string("plugins/WorldEdit/"))

namespace worldedit {

    inline std::unordered_map<std::string,class PlayerData>& getPlayersDataMap();
    class PlayerData& getPlayersData(std::string xuid);

    std::unordered_map<int, std::string>& getBlockNameMap();
    std::unordered_map<std::string, int>& getBlockIdMap();
    std::unordered_map<mce::Color, int>& getBlockColorMap();
    std::unordered_map<std::string, Block*>& getJavaBlockMap();
    std::string getBlockName(int id);
    bool isBEBlock(const std::string& s);
    bool isJEBlock(const std::string& s);
    int getBlockId(const std::string& name);
    void setFunction(std::unordered_map<::std::string, double>& variables,
                     class EvalFunctions& funcs,
                     const BoundingBox& boundingBox,
                     const Vec3& playerPos,
                     const BlockPos& pos,
                     const Vec3& center);
}  // namespace worldedit