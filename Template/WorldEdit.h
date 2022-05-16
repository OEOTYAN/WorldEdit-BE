//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#ifndef WORLDEDIT_WORLDEDIT_H
#define WORLDEDIT_WORLDEDIT_H
#include "pch.h"
#include "region/Regions.h"
#include <map>

namespace worldedit {
    class WE {
       public:
        std::map<std::string, Region*> playerRegionMap;
        std::map<std::string, std::pair<BlockPos, std::pair<int, int>>>
            playerMainPosMap;
        std::map<std::string, std::pair<BlockPos, std::pair<int, int>>>
            playerVicePosMap;
        void renderMVpos();
    };
    WE& getMod();
}  // namespace worldedit

#endif  // WORLDEDIT_WORLDEDIT_H