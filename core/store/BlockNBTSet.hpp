//
// Created by OEOTYAN on 2022/05/20.
//
#pragma once

#include "Globals.h"
#include <mc/Level.hpp>
// #include <mc/BlockInstance.hpp>
#include <mc/Block.hpp>
#include <mc/BlockActor.hpp>
#include <mc/CommandUtils.hpp>
#include <mc/BlockSource.hpp>
#include <mc/BedrockBlocks.hpp>
#include <mc/StructureSettings.hpp>
#include <mc/VanillaBlockStateTransformUtils.hpp>
#include "particle/Graphics.h"

namespace worldedit {

    class BlockNBTSet {
       public:
        Block* block = const_cast<Block*>(BedrockBlocks::mAir);
        Block* exblock = const_cast<Block*>(BedrockBlocks::mAir);
        std::string blockEntity;
        bool hasBlock = false;
        bool hasBlockEntity = false;
        bool hasBiome = false;
        int biomeId = 0;
        BlockNBTSet() = default;
        BlockNBTSet(BlockInstance& blockInstance);
        Block* getBlock() const { return block; }
        Block* getExBlock() const { return exblock; }
        bool setBlockWithoutcheckGMask(const BlockPos& pos,
                      BlockSource* blockSource,
                      class PlayerData& data, bool needBiome = false) const;
        bool setBlock(const BlockPos& pos,
                      BlockSource* blockSource,
                      class PlayerData& data,
                      class EvalFunctions& funcs,
                      phmap::flat_hash_map<std::string, double> const& var, bool needBiome = false) const;
        bool setBlock(const BlockPos& pos,
                      BlockSource* blockSource, class PlayerData& data,
                      class EvalFunctions& funcs,
                      phmap::flat_hash_map<std::string, double> const& var,
                      Rotation rotation,
                      Mirror mirror, bool needBiome = false) const;
    };
}  // namespace worldedit