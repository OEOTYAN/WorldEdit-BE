//
// Created by OEOTYAN on 2022/06/10.
//
#include "BlockNBTSet.hpp"
#include "data/PlayerData.h"

namespace worldedit {

    BlockNBTSet::BlockNBTSet(BlockInstance& blockInstance) : hasBlock(true) {
        block = blockInstance.getBlock();
        exblock = &const_cast<Block&>((blockInstance.getBlockSource()->getExtraBlock(blockInstance.getPosition())));
        if (blockInstance.hasBlockEntity()) {
            hasBlockEntity = true;
            auto be = blockInstance.getBlockEntity();
            if (be != nullptr) {
                blockEntity = be->getNbt()->toBinaryNBT();
            }
        }
    }

    bool BlockNBTSet::setBlockForHistory(const BlockPos& pos,
                               BlockSource* blockSource,
                               class PlayerData& data) const {
        if (!data.setBlockForHistory(blockSource, pos, block, exblock)) {
            return false;
        }
        if (hasBlockEntity) {
            if (block->hasBlockEntity()) {
                auto be = blockSource->getBlockEntity(pos);
                if (be != nullptr && blockEntity != "") {
                    be->setNbt(CompoundTag::fromBinaryNBT(blockEntity).get());
                }
            }
        }
        return true;
    }

    bool BlockNBTSet::setBlock(const BlockPos& pos,
                               BlockSource* blockSource,
                               class PlayerData& data,
                               class EvalFunctions& funcs,
                               std::unordered_map<std::string, double> const& var) const {
        if (!data.setBlockSimple(blockSource, funcs, var, pos, block, exblock)) {
            return false;
        }
        if (hasBlockEntity) {
            if (block->hasBlockEntity()) {
                auto be = blockSource->getBlockEntity(pos);
                if (be != nullptr && blockEntity != "") {
                    be->setNbt(CompoundTag::fromBinaryNBT(blockEntity).get());
                }
            }
        }
        return true;
    }
    bool BlockNBTSet::setBlock(const BlockPos& pos,
                               BlockSource* blockSource,
                               class PlayerData& data,
                               class EvalFunctions& funcs,
                               std::unordered_map<std::string, double> const& var,
                               Rotation rotation,
                               Mirror mirror) const {
        if (!data.setBlockSimple(blockSource, funcs, var, pos, block, exblock)) {
            return false;
        }
        if (hasBlockEntity) {
            if (block->hasBlockEntity()) {
                auto be = blockSource->getBlockEntity(pos);
                if (be != nullptr && blockEntity != "") {
                    be->setNbt(CompoundTag::fromBinaryNBT(blockEntity).get());
                }
            }
        }
        return true;
    }
}  // namespace worldedit
