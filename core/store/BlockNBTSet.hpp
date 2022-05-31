//
// Created by OEOTYAN on 2022/05/20.
//
#pragma once
#ifndef WORLDEDIT_BLOCKNBTSET_H
#define WORLDEDIT_BLOCKNBTSET_H

#include "pch.h"
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
// #include <MC/Player.hpp>
// #include <MC/ServerPlayer.hpp>
// #include <MC/Dimension.hpp>
// #include <MC/ItemStack.hpp>
// #include "Version.h"
// #include "string/StringTool.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
// #include <ScheduleAPI.h>
// #include <DynamicCommandAPI.h>
// #include "WorldEdit.h"

namespace worldedit {
    class BlockNBTSet {
       public:
        std::string block, exblock, blockEntity;
        bool hasBlock = false;
        bool hasBlockEntity = false;
        BlockNBTSet() = default;
        BlockNBTSet(BlockInstance& blockInstance) : hasBlock(true) {
            block = blockInstance.getBlock()->getNbt()->toBinaryNBT();
            exblock = const_cast<Block&>(
                          blockInstance.getBlockSource()->getExtraBlock(
                              blockInstance.getPosition()))
                          .getNbt()
                          ->toBinaryNBT();
            if (blockInstance.hasBlockEntity()) {
                hasBlockEntity = true;
                blockEntity =
                    blockInstance.getBlockEntity()->getNbt()->toBinaryNBT();
            }
        }
        Block* getBlock() const {
            return Block::create(CompoundTag::fromBinaryNBT(block).get());
        }
        Block* getExBlock() const {
            return Block::create(CompoundTag::fromBinaryNBT(exblock).get());
        }
        std::string getBlockEntity() const {
            return hasBlockEntity ? blockEntity : "";
        }
        void setBlock(const BlockPos& pos, int dimID) const;
        void setBlock(const BlockPos& pos,
                      int dimID,
                      Rotation rotation,
                      Mirror mirror) const;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_BLOCKNBTSET_H