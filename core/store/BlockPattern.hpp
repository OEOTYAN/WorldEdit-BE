//
// Created by OEOTYAN on 2022/05/18.
//
#pragma once
#ifndef WORLDEDIT_BLOCKPATTERN_H
#define WORLDEDIT_BLOCKPATTERN_H

#include "pch.h"
#include "Utils/StringHelper.h"
#include <MC/Block.hpp>
#include <MC/BedrockBlocks.hpp>
#include "WorldEdit.h"
#include "region/ChangeRegion.hpp"
// #include <EventAPI.h>
// #include <LoggerAPI.h>
// #include <MC/Level.hpp>
// #include <MC/BlockInstance.hpp>
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

namespace worldedit {
    class Percents {
       public:
        bool        isNum    = true;
        double      value    = 1;
        std::string function = "1";
        Percents()           = default;
        template <typename functions>
        double getPercents(const std::unordered_map<::std::string, double>& variables, functions& funcs) {
            if (isNum) {
                return value;
            }
            return cpp_eval::eval<double>(function.c_str(), variables, funcs);
        }
    };
    class RawBlock {
       public:
        bool        constBlock    = true;
        Block*      block         = const_cast<Block*>(BedrockBlocks::mAir);
        Block*      exBlock       = const_cast<Block*>(BedrockBlocks::mAir);
        bool        hasBE         = false;
        std::string blockEntity   = "";
        int         blockId       = -2140000000;
        int         blockData     = -2140000000;
        std::string blockIdfunc   = "0";
        std::string blockDatafunc = "0";
        RawBlock()                = default;
        template <typename functions>
        Block* getBlock(const std::unordered_map<::std::string, double>& variables, functions& funcs) {
            if (constBlock) {
                return block;
            }
            int mId = blockId;
            if (mId == -1) {
                mId = static_cast<int>(round(cpp_eval::eval<double>(blockIdfunc.c_str(), variables, funcs)));
            } else if (mId == -2140000000) {
                mId = 0;
            }
            int mData = blockData;
            if (mData == -1) {
                mData = static_cast<int>(round(cpp_eval::eval<double>(blockDatafunc.c_str(), variables, funcs)));
            } else if (mData == -2140000000) {
                mData = 0;
            }
            return Block::create(getFxxkBlockName(mId), mData);
        }
        std::string getFxxkBlockName(int id);
    };
    class BlockPattern {
       public:
        size_t                blockNum = 0;
        std::vector<Percents> percents;
        std::vector<RawBlock> rawBlocks;
        Clipboard*            clipboard = nullptr;
        BlockPos              bias      = {0, 0, 0};
        BlockPattern(std::string str, std::string xuid = "", Region* region = nullptr);
        template <typename functions>
        RawBlock* getRawBlock(const std::unordered_map<::std::string, double>& variables, functions& funcs) {
            std::vector<double> weights;
            double              total = 0;
            weights.resize(blockNum);
            for (int i = 0; i < blockNum; i++) {
                weights[i] = std::max(percents[i].getPercents(variables, funcs), 0.0);
                total += weights[i];
            }
            double random = uniformRandDouble() * total;
            double sum    = 0;
            for (int i = 0; i < blockNum - 1; i++) {
                sum += weights[i];
                if (random <= sum) {
                    return &rawBlocks[i];
                }
            }
            return &rawBlocks[blockNum - 1];
        }
        template <typename functions>
        Block* getBlock(const std::unordered_map<::std::string, double>& variables, functions& funcs) {
            return getRawBlock(variables, funcs)->getBlock(variables, funcs);
        }
        bool hasBlock(Block* block);
        template <typename functions>
        void setBlock(const std::unordered_map<::std::string, double>& variables,
                      functions&                                       funcs,
                      BlockSource*                                     blockSource,
                      const BlockPos&                                  pos) {
            if (clipboard != nullptr) {
                clipboard->getSet2(pos - bias).setBlock(pos, blockSource);
                return;
            }
            auto  blockInstance = blockSource->getBlockInstance(pos);
            auto* rawBlock      = getRawBlock(variables, funcs);
            auto* block         = rawBlock->getBlock(variables, funcs);
            setBlockSimple(blockSource, pos, block, rawBlock->exBlock);
            if (rawBlock->hasBE && blockInstance.hasBlockEntity()) {
                blockInstance.getBlockEntity()->setNbt(CompoundTag::fromBinaryNBT(rawBlock->blockEntity).get());
            }
        }
    };
}  // namespace worldedit

#endif  // WORLDEDIT_BLOCKPATTERN_H