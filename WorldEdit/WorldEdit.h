//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once

#include "Globals.h"
#include <LoggerAPI.h>
#include "data/PlayerData.h"

#define WE_DIR (std::string("plugins/WorldEdit/"))

namespace worldedit {


    Logger& logger();
    void clearBlockEntity(class BlockActor* be);
    phmap::flat_hash_map<std::string, std::unique_ptr<class PlayerData>>& getPlayersDataMap();
    class PlayerData& getPlayersData(std::string_view xuid);
    phmap::flat_hash_map<int, std::string>& getBlockNameMap();
    phmap::flat_hash_map<std::string, int>& getBlockIdMap();
    phmap::flat_hash_map<mce::Color, Block*>& getColorBlockMap();
    phmap::flat_hash_map<Block*, mce::Color>& getBlockColorMap();
    phmap::flat_hash_map<std::string, Block*>& getJavaBlockMap();
    void javaBlockMapInit();
    void blockColorMapInit();
    std::string getBlockName(int id);
    bool isBEBlock(std::string_view s);
    bool isJEBlock(std::string_view s);
    int getBlockId(std::string_view name);
    Block* tryGetBlockFromAllVersion(std::string_view name);
    void setFunction(phmap::flat_hash_map<::std::string, double>& variables,
                     class EvalFunctions& funcs,
                     const BoundingBox& boundingBox,
                     const Vec3& playerPos,
                     const BlockPos& pos,
                     const Vec3& center);
}  // namespace worldedit