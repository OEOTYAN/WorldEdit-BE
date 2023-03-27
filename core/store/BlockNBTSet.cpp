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

    BlockNBTSet::BlockNBTSet(BlockNBTSet const& other) {
        if (other.blockEntity)
            blockEntity = other.blockEntity->clone();
        blocks = other.blocks;
        biomeId = other.biomeId;
    }

    BlockNBTSet& BlockNBTSet::operator=(BlockNBTSet const& other) {
        if (other.blockEntity)
            blockEntity = other.blockEntity->clone();
        blocks = other.blocks;
        biomeId = other.biomeId;
        return *this;
    }
    BlockNBTSet::BlockNBTSet(BlockInstance& blockInstance) {
        auto* bs = blockInstance.getBlockSource();
        auto pos = blockInstance.getPosition();
        blocks = {blockInstance.getBlock(), const_cast<Block*>(&bs->getExtraBlock(pos))};
        // auto* biome = bs->tryGetBiome(pos);
        // if (biome != nullptr) {
        //     biomeId = biome->getId();
        // }
        if (blockInstance.hasBlockEntity()) {
            auto be = blockInstance.getBlockEntity();
            if (be != nullptr) {
                blockEntity = be->getNbt();
            }
        }
    }

    bool BlockNBTSet::setBlockWithoutcheckGMask(const BlockPos& pos,
                                                BlockSource* blockSource,
                                                class PlayerData& data,
                                                bool setBiome) const {
        auto& [block, exblock] = blocks.value();
        bool res;
        res = data.setBlockWithoutcheckGMask(blockSource, pos, block, exblock, setBiome ? biomeId : std::nullopt);
        if (blockEntity != nullptr && block->hasBlockEntity()) {
            auto be = blockSource->getBlockEntity(pos);
            if (be != nullptr) {
                return be->setNbt(blockEntity.get());
            } else {
                LevelChunk* chunk = blockSource->getChunkAt(pos);
                if (chunk != nullptr) {
                    auto b = BlockActor::create(blockEntity.get());
                    if (b != nullptr) {
                        b->moveTo(pos);
                        chunk->_placeBlockEntity(b);
                    }
                }
            }
        }
        return res;
    }

    bool BlockNBTSet::setBlock(const BlockPos& pos,
                               BlockSource* blockSource,
                               class PlayerData& data,
                               class EvalFunctions& funcs,
                               phmap::flat_hash_map<std::string, double> const& var,
                               bool setBiome) const {
        auto& [block, exblock] = blocks.value();
        bool res;
        res = data.setBlockSimple(blockSource, funcs, var, pos, block, exblock, setBiome ? biomeId : std::nullopt);
        if (blockEntity != nullptr && block->hasBlockEntity()) {
            auto be = blockSource->getBlockEntity(pos);
            if (be != nullptr) {
                return be->setNbt(blockEntity.get());
            } else {
                LevelChunk* chunk = blockSource->getChunkAt(pos);
                if (chunk != nullptr) {
                    auto b = BlockActor::create(blockEntity.get());
                    if (b != nullptr) {
                        b->moveTo(pos);
                        chunk->_placeBlockEntity(b);
                    }
                }
            }
        }
        return res;
    }
    bool BlockNBTSet::setBlock(const BlockPos& pos,
                               BlockSource* blockSource,
                               class PlayerData& data,
                               class EvalFunctions& funcs,
                               phmap::flat_hash_map<std::string, double> const& var,
                               Rotation rotation,
                               Mirror mirror,
                               bool setBiome) const {
        auto& [block, exblock] = blocks.value();
        bool res;
        res = data.setBlockSimple(
            blockSource, funcs, var, pos,
            const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(*block, rotation, mirror)),
            const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(*exblock, rotation, mirror)),
            setBiome ? biomeId : std::nullopt);
        if (blockEntity != nullptr && block->hasBlockEntity()) {
            auto be = blockSource->getBlockEntity(pos);
            if (be != nullptr) {
                return be->setNbt(blockEntity.get());
            } else {
                LevelChunk* chunk = blockSource->getChunkAt(pos);
                if (chunk != nullptr) {
                    auto b = BlockActor::create(blockEntity.get());
                    if (b != nullptr) {
                        b->moveTo(pos);
                        chunk->_placeBlockEntity(b);
                    }
                }
            }
        }
        return res;
    }
}  // namespace worldedit
