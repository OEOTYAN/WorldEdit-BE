//
// Created by OEOTYAN on 2022/05/18.
//
#pragma once
#ifndef WORLDEDIT_BLOCKPATTERN_H
#define WORLDEDIT_BLOCKPATTERN_H

#include "pch.h"
#include "Utils/StringHelper.h"
// #include <EventAPI.h>
// #include <LoggerAPI.h>
// #include <MC/Level.hpp>
// #include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BedrockBlocks.hpp>
// #include <MC/BlockSource.hpp>
// #include <MC/Actor.hpp>
// #include <MC/Player.hpp>
// #include <MC/ServerPlayer.hpp>
// #include <MC/Dimension.hpp>
// #include <MC/ItemStack.hpp>
// #include "Version.h"
// #include "eval/Eval.h"
// #include "string/StringTool.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
// #include <ScheduleAPI.h>
// #include <DynamicCommandAPI.h>
#include "WorldEdit.h"

namespace worldedit {
    class Percents {
       public:
        bool isNum = true;
        double value = 1;
        std::string function = "1";
        Percents() = default;
        template <typename functions>
        double getPercents(
            const std::unordered_map<::std::string, double>& variables,
            functions& funcs) {
            if (isNum) {
                return value;
            }
            return cpp_eval::eval<double>(function.c_str(), variables, funcs);
        }
    };
    class rawBlock {
       public:
        bool constBlock = true;
        Block* block = const_cast<Block*>(BedrockBlocks::mAir);
        int blockId = 0;
        int blockData = 0;
        std::string blockIdfunc = "0";
        std::string blockDatafunc = "0";
        rawBlock() = default;
        template <typename functions>
        Block* getBlock(
            const std::unordered_map<::std::string, double>& variables,
            functions& funcs) {
            if (constBlock) {
                return block;
            }
            int mId = blockId;
            if (mId < 0) {
                mId = static_cast<int>(round(cpp_eval::eval<double>(
                    blockIdfunc.c_str(), variables, funcs)));
            }
            int mData = blockData;
            if (mData < 0) {
                mData = static_cast<int>(round(cpp_eval::eval<double>(
                    blockDatafunc.c_str(), variables, funcs)));
            }
            return Block::create(getBlockName(mId), mData);
        }
    };
    class BlockPattern {
       public:
        size_t blockNum = 0;
        std::vector<Percents> percents;
        std::vector<rawBlock> rawBlocks;
        BlockPattern(std::string str) {
            std::vector<std::string> raw;
            raw.resize(0);
            string form = "";
            size_t i = 0;
            while (i < str.size()) {
                if (str[i] == '-' || isdigit(str[i])) {
                    auto head = i;
                    i++;
                    while (i < str.size() &&
                           (str[i] == '.' || isdigit(str[i]))) {
                        i++;
                    }
                    form += string("num") + str[i];
                    raw.emplace_back(str.substr(head, i - head));
                } else if (str[i] == '\'') {
                    i++;
                    auto head = i;
                    while (i < str.size() && (str[i] != '\'')) {
                        i++;
                    }
                    raw.emplace_back(str.substr(head, i - head));
                    i++;
                    form += string("funciton") + str[i];
                } else if (isalpha(str[i])) {
                    auto head = i;
                    i++;
                    while (i < str.size() &&
                           (isalpha(str[i]) || str[i] == '_' ||
                            isdigit(str[i]) ||
                            (str[i] == ':' && i + 1 < str.size() &&
                             isalpha(str[i + 1])))) {
                        i++;
                    }
                    form += string("block") + str[i];
                    raw.emplace_back(str.substr(head, i - head));
                } else if (str[i] == '{') {
                    auto head = i;
                    int bracket = -1;
                    i++;
                    while (i < str.size() && bracket < 0) {
                        if (str[i] == '{') {
                            bracket--;
                        } else if (str[i] == '}') {
                            bracket++;
                        }
                        i++;
                    }
                    form += string("SNBT") + str[i];
                    raw.emplace_back(str.substr(head, i - head));
                }
                i++;
            }
            auto blockList = SplitStrWithPattern(form, ",");
            blockNum = blockList.size();
            percents.resize(blockNum);
            rawBlocks.resize(blockNum);
            int rawPtr = 0;
            for (int iter = 0; iter < blockNum; iter++) {
                if (blockList[iter].find("%") != std::string::npos) {
                    if (blockList[iter].find("num%") != std::string::npos) {
                        percents[iter].value = std::stod(raw[rawPtr]);
                    } else {
                        percents[iter].isNum = false;
                        percents[iter].function = raw[rawPtr];
                    }
                    rawPtr++;
                } else {
                    blockList[iter] = "%" + blockList[iter];
                }
                if (blockList[iter].find("%num") != std::string::npos) {
                    rawBlocks[iter].blockId = std::stoi(raw[rawPtr]);
                    rawPtr++;
                } else if (blockList[iter].find("%block") !=
                           std::string::npos) {
                    std::string tmpName = raw[rawPtr];
                    if (tmpName.find("minecraft:") == std::string::npos) {
                        tmpName = "minecraft:" + tmpName;
                    }
                    rawBlocks[iter].blockId = getBlockId(tmpName);
                    rawPtr++;
                } else if (blockList[iter].find("%funciton") !=
                           std::string::npos) {
                    rawBlocks[iter].blockId = -1;
                    rawBlocks[iter].constBlock = false;
                    rawBlocks[iter].blockIdfunc = raw[rawPtr];
                    rawPtr++;
                } else if (blockList[iter].find("%SNBT") != std::string::npos) {
                    rawBlocks[iter].blockId = -1;
                    rawBlocks[iter].blockData = -1;
                    rawBlocks[iter].block =
                        Block::create(CompoundTag::fromSNBT(raw[rawPtr]).get());
                    rawPtr++;
                    continue;
                }
                if (blockList[iter].find(":num") != std::string::npos) {
                    rawBlocks[iter].blockData = std::stoi(raw[rawPtr]);
                    rawPtr++;
                } else if (blockList[iter].find(":funciton") !=
                           std::string::npos) {
                    rawBlocks[iter].blockData = -1;
                    rawBlocks[iter].constBlock = false;
                    rawBlocks[iter].blockDatafunc = raw[rawPtr];
                    rawPtr++;
                }
                if (rawBlocks[iter].constBlock) {
                    rawBlocks[iter].block =
                        Block::create(getBlockName(rawBlocks[iter].blockId),
                                      rawBlocks[iter].blockData);
                }
            }
        }
        template <typename functions>
        Block* getBlock(
            const std::unordered_map<::std::string, double>& variables,
            functions& funcs) {
            std::vector<double> weights;
            double total = 0;
            weights.resize(blockNum);
            for (int i = 0; i < blockNum; i++) {
                weights[i] =
                    std::max(percents[i].getPercents(variables, funcs), 0.0);
                total += weights[i];
            }
            double random = uniform_dist(rng) * total;
            double sum = 0;
            for (int i = 0; i < blockNum - 1; i++) {
                sum += weights[i];
                if (random <= sum) {
                    return rawBlocks[i].getBlock(variables, funcs);
                }
            }
            return rawBlocks[blockNum - 1].getBlock(variables, funcs);
        }
    };
}  // namespace worldedit

#endif  // WORLDEDIT_BLOCKPATTERN_H