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
#include "tool/VanillaFuckMojangBlocks.h"
#include "brush/Brush.h"

#define WE_DIR (std::string("plugins/WorldEdit/"))

#define INNERIZE_GMASK                                                                                                \
    std::function<void(EvalFunctions&, std::unordered_map<std::string, double> const&, std::function<void()> const&)> \
        gMaskLambda;                                                                                                  \
    std::string gMask = "";                                                                                           \
    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {                                                  \
        gMask = mod.playerGMaskMap[xuid];                                                                             \
        gMaskLambda = [&](EvalFunctions& func, const std::unordered_map<std::string, double>& var,                    \
                          std::function<void()> const& todo) mutable {                                                \
            if (cpp_eval::eval<double>(gMask, var, func) > 0.5) {                                             \
                todo();                                                                                               \
            }                                                                                                         \
        };                                                                                                            \
    } else {                                                                                                          \
        gMaskLambda = [&](EvalFunctions& func, const std::unordered_map<std::string, double>& var,                    \
                          std::function<void()> const& todo) mutable { todo(); };                                     \
    }

namespace worldedit {
    class WE {
       public:
        int maxHistoryLength = 20;
        int updateArg = 2;
        int updateExArg = 1;
        std::unordered_map<std::string, Region*> playerRegionMap;
        std::unordered_map<std::string, std::pair<BlockPos, std::pair<int, int>>> playerMainPosMap;
        std::unordered_map<std::string, std::pair<BlockPos, std::pair<int, int>>> playerVicePosMap;
        std::unordered_map<std::string, Clipboard> playerClipboardMap;
        std::unordered_map<std::string, std::unordered_map<std::string, Tool*>> playerHandToolMap;
        std::unordered_map<std::string, std::unordered_map<std::string, Brush*>> playerBrushMap;
        std::unordered_map<std::string, std::string> playerGMaskMap;
        std::unordered_map<std::string, std::pair<std::vector<Clipboard>, std::pair<int, int>>> playerHistoryMap;

        void renderMVpos();
        Clipboard* getPlayerNextHistory(std::string xuid);
        std::pair<Clipboard*, int> getPlayerUndoHistory(std::string xuid);
        std::pair<Clipboard*, int> getPlayerRedoHistory(std::string xuid);
    };
    WE& getMod();
    std::unordered_map<int, std::string>& getBlockNameMap();
    std::unordered_map<std::string, int>& getBlockIdMap();
    std::unordered_map<mce::Color, int>& getBlockColorMap();
    Block* getBlock(int id);
    std::string getBlockName(int id);
    int getBlockId(const std::string& name);
}  // namespace worldedit

// std::unordered_map<std::string, std::string>& getBlockColorssMap();
#endif  // WORLDEDIT_WORLDEDIT_H