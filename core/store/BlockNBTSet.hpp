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
            block = blockInstance.getBlock()->getNbt()->toSNBT(
                0, SnbtFormat::Minimize);
            exblock = const_cast<Block&>(
                          blockInstance.getBlockSource()->getExtraBlock(
                              blockInstance.getPosition()))
                          .getNbt()
                          ->toSNBT(0, SnbtFormat::Minimize);
            if (blockInstance.hasBlockEntity()) {
                hasBlockEntity = true;
                blockEntity = blockInstance.getBlockEntity()->getNbt()->toSNBT(
                    0, SnbtFormat::Minimize);
            }
        }
        Block* getBlock() const {
            return Block::create(CompoundTag::fromSNBT(block).get());
        }
        Block* getExBlock() const {
            return Block::create(CompoundTag::fromSNBT(exblock).get());
        }
        std::string getBlockEntity() const {
            return hasBlockEntity ? blockEntity : "";
        }
        void setBlock(BlockPos& pos, int dimID) const {
            auto blockSource = Level::getBlockSource(dimID);
            blockSource->setBlock(
                pos, *Block::create(CompoundTag::fromSNBT(block).get()), 2,
                nullptr, nullptr);
            blockSource->setExtraBlock(
                pos, *Block::create(CompoundTag::fromSNBT(exblock).get()), 2);
            if (hasBlockEntity) {
                auto blockInstance = blockSource->getBlockInstance(pos);
                if (blockInstance.hasBlockEntity()) {
                    blockInstance.getBlockEntity()->setNbt(
                        CompoundTag::fromSNBT(blockEntity).get());
                }
            }
        }
        std::string getStr() {
            return "block:\n" + block + "\nexblock:\n" + exblock +
                   (hasBlockEntity ? ("\nblockEntity:\n" + blockEntity) : "");
        }
    };
}  // namespace worldedit

#endif  // WORLDEDIT_BLOCKNBTSET_H