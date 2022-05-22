//
// Created by OEOTYAN on 2022/05/20.
//
#pragma once
#ifndef WORLDEDIT_BLOCKNBTSET_H
#define WORLDEDIT_BLOCKNBTSET_H

#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockActor.hpp>
#include <MC/BlockSource.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/StructureSettings.hpp>
#include <MC/VanillaBlockStateTransformUtils.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/Dimension.hpp>
#include <MC/ItemStack.hpp>
#include "Version.h"
#include "string/StringTool.h"
#include <LLAPI.h>
#include <ServerAPI.h>
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include <DynamicCommandAPI.h>
#include "WorldEdit.h"

namespace worldedit {
    class blockNBTSet {
       public:
        std::string block, exblock, blockEntity;
        bool hasBlock = false;
        bool hasBlockEntity = false;
        blockNBTSet() = default;
        blockNBTSet(BlockInstance& blockInstance) : hasBlock(true) {
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
        void setBlock(BlockPos& pos, int dimID) const {
            auto blockSource = Level::getBlockSource(dimID);
            blockSource->setBlock(
                pos, *Block::create(CompoundTag::fromBinaryNBT(block).get()), 2,
                nullptr, nullptr);
            blockSource->setExtraBlock(
                pos, *Block::create(CompoundTag::fromBinaryNBT(exblock).get()),
                2);
            if (hasBlockEntity) {
                auto blockInstance = blockSource->getBlockInstance(pos);
                if (blockInstance.hasBlockEntity()) {
                    blockInstance.getBlockEntity()->setNbt(
                        CompoundTag::fromBinaryNBT(blockEntity).get());
                }
            }
        }
        void setBlock(BlockPos& pos,
                      int dimID,
                      Rotation rotation,
                      Mirror mirror) const {
            auto blockSource = Level::getBlockSource(dimID);
            blockSource->setBlock(
                pos,
                *VanillaBlockStateTransformUtils::transformBlock(
                    *Block::create(CompoundTag::fromBinaryNBT(block).get()),
                    rotation, mirror),
                2, nullptr, nullptr);
            blockSource->setExtraBlock(
                pos,
                *VanillaBlockStateTransformUtils::transformBlock(
                    *Block::create(CompoundTag::fromBinaryNBT(exblock).get()),
                    rotation, mirror),
                2);
            if (hasBlockEntity) {
                auto blockInstance = blockSource->getBlockInstance(pos);
                if (blockInstance.hasBlockEntity()) {
                    blockInstance.getBlockEntity()->setNbt(
                        CompoundTag::fromBinaryNBT(blockEntity).get());
                }
            }
        }
    };
}  // namespace worldedit

#endif  // WORLDEDIT_BLOCKNBTSET_H