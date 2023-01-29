//
// Created by OEOTYAN on 2022/05/18.
//
#pragma once
#ifndef WORLDEDIT_BLOCKPATTERN_H
#define WORLDEDIT_BLOCKPATTERN_H

#include "Global.h"
#include "eval/Eval.h"

namespace worldedit {
    class Percents {
       public:
        bool isNum = true;
        double value = 1;
        std::string function = "1";
        Percents() = default;

        double getPercents(const std::unordered_map<::std::string, double>& variables, class EvalFunctions& funcs);
    };
    class RawBlock {
       public:
        bool constBlock = true;
        class Block* block;
        class Block* exBlock;
        bool hasBE = false;
        std::string blockEntity = "";
        int blockId = -2140000000;
        int blockData = -2140000000;
        std::string blockIdfunc = "0";
        std::string blockDatafunc = "0";
        RawBlock();

        class Block* getBlock(const std::unordered_map<::std::string, double>& variables, class EvalFunctions& funcs);
    };
    class BlockPattern {
       public:
        size_t blockNum = 0;
        std::vector<Percents> percents;
        std::vector<RawBlock> rawBlocks;
        class Clipboard* clipboard = nullptr;
        class PlayerData* playerData = nullptr;
        BlockPos bias = {0, 0, 0};
        BlockPattern(std::string str, std::string xuid = "", class Region* region = nullptr);

        RawBlock* getRawBlock(const std::unordered_map<::std::string, double>& variables, class EvalFunctions& funcs);

        class Block* getBlock(const std::unordered_map<::std::string, double>& variables, class EvalFunctions& funcs);

        bool hasBlock(class Block* block);

        bool setBlock(const std::unordered_map<::std::string, double>& variables,
                      class EvalFunctions& funcs,
                      BlockSource* blockSource,
                      const BlockPos& pos);
    };
}  // namespace worldedit

#endif  // WORLDEDIT_BLOCKPATTERN_H