//
// Created by OEOTYAN on 2022/06/10.
//
#include "BlockNBTSet.hpp"
#include "data/PlayerData.h"
#include "WorldEdit.h"
#include <mc/BlockActor.hpp>
#include <mc/LevelChunk.hpp>
#include <mc/Biome.hpp>

namespace worldedit {

    BlockNBTSet::BlockNBTSet(BlockInstance& blockInstance) : hasBlock(true) {
        block = blockInstance.getBlock();
        auto* bs = blockInstance.getBlockSource();
        auto pos = blockInstance.getPosition();
        exblock = &const_cast<Block&>((bs->getExtraBlock(pos)));
        auto* biome = bs->tryGetBiome(pos);
        if (biome != nullptr) {
            hasBiome = true;
            biomeId = biome->getId();
        }
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
                                                class PlayerData& data,
                                                bool setBiome) const {
        bool res;
        if (setBiome && hasBiome) {
            res = data.setBlockWithBiomeWithoutcheckGMask(blockSource, pos, block, exblock, biomeId);
        } else {
            res = data.setBlockWithoutcheckGMask(blockSource, pos, block, exblock);
        }
        if (hasBlockEntity && blockEntity != "" && block->hasBlockEntity()) {
            auto be = blockSource->getBlockEntity(pos);
            if (be != nullptr) {
                return be->setNbt(CompoundTag::fromBinaryNBT(blockEntity).get());
            } else {
                LevelChunk* chunk = blockSource->getChunkAt(pos);
                auto b = BlockActor::create(CompoundTag::fromBinaryNBT(blockEntity).get());
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
                               bool setBiome) const {
        bool res;
        if (setBiome && hasBiome) {
            res = data.setBlockWithBiomeSimple(blockSource, funcs, var, pos, block, exblock, biomeId);
        } else {
            res = data.setBlockSimple(blockSource, funcs, var, pos, block, exblock);
        }
        if (hasBlockEntity && blockEntity != "" && block->hasBlockEntity()) {
            auto be = blockSource->getBlockEntity(pos);
            if (be != nullptr) {
                return be->setNbt(CompoundTag::fromBinaryNBT(blockEntity).get());
            } else {
                LevelChunk* chunk = blockSource->getChunkAt(pos);
                auto b = BlockActor::create(CompoundTag::fromBinaryNBT(blockEntity).get());
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
                               Mirror mirror,
                               bool setBiome) const {
        bool res;
        if (setBiome && hasBiome) {
            res = data.setBlockWithBiomeSimple(
                blockSource, funcs, var, pos,
                const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(*block, rotation, mirror)),
                const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(*exblock, rotation, mirror)),
                biomeId);
        } else {
            res = data.setBlockSimple(
                blockSource, funcs, var, pos,
                const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(*block, rotation, mirror)),
                const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(*exblock, rotation, mirror)));
        }
        if (hasBlockEntity && blockEntity != "" && block->hasBlockEntity()) {
            auto be = blockSource->getBlockEntity(pos);
            if (be != nullptr) {
                return be->setNbt(CompoundTag::fromBinaryNBT(blockEntity).get());
            } else {
                LevelChunk* chunk = blockSource->getChunkAt(pos);
                auto b = BlockActor::create(CompoundTag::fromBinaryNBT(blockEntity).get());
                if (b != nullptr) {
                    b->moveTo(pos);
                    chunk->_placeBlockEntity(b);
                }
            }
        }
        return res;
    }
}  // namespace worldedit
