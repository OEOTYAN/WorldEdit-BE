//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once

#include "Global.h"
#include <LoggerAPI.h>
#include "data/PlayerData.h"

#define WE_DIR (std::string("plugins/WorldEdit/"))

namespace worldedit {

    Logger& logger();
    void clearBlockEntity(class BlockActor* be);
    std::unordered_map<std::string,class PlayerData>& getPlayersDataMap();
    class PlayerData& getPlayersData(std::string xuid);
    std::unordered_map<int, std::string>& getBlockNameMap();
    std::unordered_map<std::string, int>& getBlockIdMap();
    std::unordered_map<mce::Color, Block*>& getColorBlockMap();
    std::unordered_map<Block *,mce::Color>& getBlockColorMap();
    std::unordered_map<std::string, Block*>& getJavaBlockMap();
    void javaBlockMapInit();
    void blockColorMapInit();
    std::string getBlockName(int id);
    bool isBEBlock(const std::string& s);
    bool isJEBlock(const std::string& s);
    int getBlockId(const std::string& name);
    Block* tryGetBlockFromAllVersion(const std::string& name);
    void setFunction(std::unordered_map<::std::string, double>& variables,
                     class EvalFunctions& funcs,
                     const BoundingBox& boundingBox,
                     const Vec3& playerPos,
                     const BlockPos& pos,
                     const Vec3& center);
}  // namespace worldedit