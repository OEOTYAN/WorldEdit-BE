//
// Created by OEOTYAN on 2022/05/20.
//
#pragma once
#ifndef WORLDEDIT_BLOCKNBTSET_H
#define WORLDEDIT_BLOCKNBTSET_H

#include "Global.h"
#include <MC/Level.hpp>
// #include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockActor.hpp>
#include <MC/CommandUtils.hpp>
#include <MC/BlockSource.hpp>
#include <MC/BedrockBlocks.hpp>
#include <MC/StructureSettings.hpp>
#include <MC/VanillaBlockStateTransformUtils.hpp>
#include "particle/Graphics.h"

namespace worldedit {

    class BlockNBTSet {
       public:
        Block* block = const_cast<Block*>(BedrockBlocks::mAir);
        Block* exblock = const_cast<Block*>(BedrockBlocks::mAir);
        std::string blockEntity;
        bool hasBlock = false;
        bool hasBlockEntity = false;
        BlockNBTSet() = default;
        BlockNBTSet(BlockInstance& blockInstance);
        Block* getBlock() const { return block; }
        Block* getExBlock() const { return exblock; }
        bool setBlockForHistory(const BlockPos& pos,
                      BlockSource* blockSource,
                      class PlayerData& data) const;
        bool setBlock(const BlockPos& pos,
                      BlockSource* blockSource,
                      class PlayerData& data,
                      class EvalFunctions& funcs,
                      std::unordered_map<std::string, double> const& var) const;
        bool setBlock(const BlockPos& pos,
                      BlockSource* blockSource, class PlayerData& data,
                      class EvalFunctions& funcs,
                      std::unordered_map<std::string, double> const& var,
                      Rotation rotation,
                      Mirror mirror) const;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_BLOCKNBTSET_H