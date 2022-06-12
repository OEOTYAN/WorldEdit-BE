//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#ifndef WORLDEDIT_WORLDEDIT_H
#define WORLDEDIT_WORLDEDIT_H

#include "pch.h"
#include "region/Regions.h"
#include "store/BlockNBTSet.hpp"
#include "store/Clipboard.hpp"
#include "tool/tool.h"
#include "brush/Brush.h"

#define WE_DIR "plugins/WorldEdit/"
#define WE_PNG_DIR "plugins/WorldEdit/image/"

namespace worldedit {
    class WE {
       public:
        int maxHistoryLength = 20;
        int updateArg = 2;
        std::unordered_map<std::string, Region*> playerRegionMap;
        std::unordered_map<std::string,
                           std::pair<BlockPos, std::pair<int, int>>>
            playerMainPosMap;
        std::unordered_map<std::string,
                           std::pair<BlockPos, std::pair<int, int>>>
            playerVicePosMap;
        std::unordered_map<std::string, Clipboard> playerClipboardMap;
        std::unordered_map<std::string, std::unordered_map<std::string, Tool*>>
            playerHandToolMap;
        std::unordered_map<std::string, std::unordered_map<std::string,Brush*>>
            playerBrushMap;
        std::unordered_map<std::string, std::string> playerGMaskMap;
        std::unordered_map<
            std::string,
            std::pair<std::vector<Clipboard>, std::pair<int, int>>>
            playerHistoryMap;

        void renderMVpos();
        Clipboard* getPlayerNextHistory(std::string xuid);
        std::pair<Clipboard*, int> getPlayerUndoHistory(std::string xuid);
        std::pair<Clipboard*, int> getPlayerRedoHistory(std::string xuid);
    };
    WE& getMod();
    std::unordered_map<int, std::string>& getBlockNameMap();
    std::unordered_map<std::string, int>& getBlockIdMap();
    Block* getBlock(int id);
    std::string getBlockName(int id);
    int getBlockId(const std::string& name);
}  // namespace worldedit

#endif  // WORLDEDIT_WORLDEDIT_H