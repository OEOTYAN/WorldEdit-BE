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
#include <map>

namespace worldedit {
    class WE {
       public:
        int historyLengyh = 20;
        std::map<std::string, Region*> playerRegionMap;
        std::map<std::string, std::pair<BlockPos, std::pair<int, int>>>
            playerMainPosMap;
        std::map<std::string, std::pair<BlockPos, std::pair<int, int>>>
            playerVicePosMap;
        std::map<std::string, Clipboard> playerClipboardMap;
        std::map<std::string, std::map<std::string, std::string>>
            playerHandToolMap;
        std::map<std::string, std::string> playerGMaskMap;
        std::map<std::string, std::pair<std::vector<Clipboard>, int>>
            playerHistoryMap;
            
        void renderMVpos();
    };
    WE& getMod();
    std::unordered_map<int, std::string>& getBlockNameMap();
    Block* getBlock(int id);
    std::string getBlockName(int id);
}  // namespace worldedit

#endif  // WORLDEDIT_WORLDEDIT_H