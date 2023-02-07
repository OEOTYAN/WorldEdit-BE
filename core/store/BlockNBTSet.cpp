//
// Created by OEOTYAN on 2022/06/10.
//
#include "BlockNBTSet.hpp"
#include "data/PlayerData.h"
#include "WorldEdit.h"
#include <mc/BlockActor.hpp>
#include <mc/LevelChunk.hpp>

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

    bool BlockNBTSet::setBlockWithoutcheckGMask(const BlockPos& pos,
                                                BlockSource* blockSource,
                                                class PlayerData& data) const {
        bool res = data.setBlockWithoutcheckGMask(blockSource, pos, block, exblock);
        if (hasBlockEntity && blockEntity != "" && block->hasBlockEntity()) {
            auto be = blockSource->getBlockEntity(pos);
            if (be != nullptr) {
                return be->setNbt(CompoundTag::fromBinaryNBT(blockEntity).get());
            } else {
                LevelChunk* chunk = blockSource->getChunkAt(pos);
                void* vtbl = dlsym("??_7DefaultDataLoadHelper@@6B@");
                auto b = BlockActor::loadStatic(*Global<Level>, *CompoundTag::fromBinaryNBT(blockEntity).get(),
                                                (class DataLoadHelper&)vtbl);
                if (b != nullptr) {
                    b->moveTo(pos);
                    chunk->_placeBlockEntity(b);
                }
            }
        }
        return res;
    }

    bool BlockNBTSet::setBlock(const BlockPos& pos,
                               BlockSource* blockSource,
                               class PlayerData& data,
                               class EvalFunctions& funcs,
                               std::unordered_map<std::string, double> const& var) const {
        bool res = data.setBlockSimple(blockSource, funcs, var, pos, block, exblock);
        if (hasBlockEntity && blockEntity != "" && block->hasBlockEntity()) {
            auto be = blockSource->getBlockEntity(pos);
            if (be != nullptr) {
                return be->setNbt(CompoundTag::fromBinaryNBT(blockEntity).get());
            } else {
                LevelChunk* chunk = blockSource->getChunkAt(pos);
                void* vtbl = dlsym("??_7DefaultDataLoadHelper@@6B@");
                auto b = BlockActor::loadStatic(*Global<Level>, *CompoundTag::fromBinaryNBT(blockEntity).get(),
                                                (class DataLoadHelper&)vtbl);
                if (b != nullptr) {
                    b->moveTo(pos);
                    chunk->_placeBlockEntity(b);
                }
            }
        }
        return res;
    }
    bool BlockNBTSet::setBlock(const BlockPos& pos,
                               BlockSource* blockSource,
                               class PlayerData& data,
                               class EvalFunctions& funcs,
                               std::unordered_map<std::string, double> const& var,
                               Rotation rotation,
                               Mirror mirror) const {
        bool res = data.setBlockSimple(
            blockSource, funcs, var, pos,
            const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(*block, rotation, mirror)),
            const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(*exblock, rotation, mirror)));
        if (hasBlockEntity && blockEntity != "" && block->hasBlockEntity()) {
            auto be = blockSource->getBlockEntity(pos);
            if (be != nullptr) {
                return be->setNbt(CompoundTag::fromBinaryNBT(blockEntity).get());
            } else {
                LevelChunk* chunk = blockSource->getChunkAt(pos);
                void* vtbl = dlsym("??_7DefaultDataLoadHelper@@6B@");
                auto b = BlockActor::loadStatic(*Global<Level>, *CompoundTag::fromBinaryNBT(blockEntity).get(),
                                                (class DataLoadHelper&)vtbl);
                if (b != nullptr) {
                    b->moveTo(pos);
                    chunk->_placeBlockEntity(b);
                }
            }
        }
        return res;
    }
}  // namespace worldedit
