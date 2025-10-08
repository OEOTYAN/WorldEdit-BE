#include "InplaceBuilder.h"
#include "utils/BlockUtils.h"
#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/actor/BlockActor.h>
#include <mc/world/level/chunk/ChunkSource.h>
#include <mc/world/level/chunk/LevelChunk.h>


namespace we {

void InplaceBuilder::fireBlockChanged(
    BlockSource&     source,
    BlockPos const&  pos,
    BlockLayer::Type layer,
    Block const&     oldBlock,
    Block const&     newBlock
) const {
    if ((updateFlags & BlockUpdateFlag::Neighbors) != 0) {
        source.updateNeighborsAt(pos);
        source.fireBlockChanged(
            pos,
            layer,
            newBlock,
            oldBlock,
            updateFlags,
            BlockChangedEventTarget::NeighborBlock,
            nullptr,
            nullptr
        );
    }
    if ((updateFlags & BlockUpdateFlag::Network) != 0) {
        source.fireBlockChanged(
            pos,
            layer,
            newBlock,
            oldBlock,
            updateFlags,
            BlockChangedEventTarget::SelfBlock,
            nullptr,
            nullptr
        );
    }
}

bool InplaceBuilder::setBlock(
    BlockSource&                source,
    BlockPos const&             pos,
    Block const&                block,
    std::shared_ptr<BlockActor> blockActor
) {
    auto chunk   = source.getChunkSource().getExistingChunk(ChunkPos{pos});
    bool applied = false;
    if (!chunk
        || chunk->mLoadState->load(std::memory_order_relaxed)
               <= ChunkState::CheckingForReplacementData) {
        return applied;
    }
    ChunkBlockPos chunkBlockPos{pos, source.getMinHeight()};

    if (auto iter = chunk->mBlockEntities->mMap->find(chunkBlockPos);
        iter != chunk->mBlockEntities->mMap->end()) {
        if (iter->second->mType != block.getBlockType().mBlockEntityType) {
            applied = true;
            chunk->mBlockEntities->mMap->erase(iter);
        } else if (blockActor) {
            // Update existing block entity with new data
            iter->second = std::move(blockActor);
            source.fireBlockEntityChanged(*iter->second);
            blockActor = nullptr; // Prevent placing a new block entity
        }
    }

    {
        // for medium
        Block const* extra;
        if (auto medium = block.getBlockType().getRequiredMedium(); medium) [[unlikely]] {
            extra = Block::tryGetFromRegistry(*medium);
        } else {
            extra = &DirectAccessBlocks::Air();
        }
        auto& previousBlock = chunk->setExtraBlock(chunkBlockPos, *extra, &source);
        if (previousBlock != *extra) {
            applied = true;
            fireBlockChanged(source, pos, BlockLayer::Extra, previousBlock, *extra);
        }
    }

    auto& previousBlock =
        chunk->setBlock(chunkBlockPos, block, &source, std::move(blockActor));
    if (previousBlock != block) {
        applied           = true;
        auto& borderBlock = source.getLevel().getRegisteredBorderBlock();
        if (borderBlock.mNameInfo->mFullName->getHash()
            != BedrockBlockNames::Air().mStrHash) {
            bool currentIsBorder = &borderBlock == &block.getBlockType();
            if (&borderBlock == &previousBlock.getBlockType() || currentIsBorder) {
                auto& ref =
                    chunk->mBorderBlockMap->at(chunkBlockPos.x + (chunkBlockPos.z << 4));
                auto previousIsBorder = ref;
                if (previousIsBorder != currentIsBorder) {
                    ref            = currentIsBorder;
                    auto totalTime = chunk->mFullChunkDirtyTicksCounters[4]->totalTime;
                    if (totalTime < 0) {
                        totalTime                                         = 0;
                        chunk->mFullChunkDirtyTicksCounters[4]->totalTime = 0;
                    }
                    chunk->mFullChunkDirtyTicksCounters[4]->lastChange = totalTime;
                }
            }
        }
        fireBlockChanged(source, pos, BlockLayer::Standard, previousBlock, block);
    }
    return applied;
}

bool InplaceBuilder::setExtraBlock(
    BlockSource&    source,
    BlockPos const& pos,
    Block const&    block
) {
    auto chunk = source.getChunkSource().getExistingChunk(ChunkPos{pos});
    if (!chunk
        || chunk->mLoadState->load(std::memory_order_relaxed)
               <= ChunkState::CheckingForReplacementData) {
        return false;
    }

    auto& previousBlock =
        chunk->setExtraBlock(ChunkBlockPos{pos, source.getMinHeight()}, block, &source);
    if (previousBlock != block) {
        fireBlockChanged(source, pos, BlockLayer::Extra, previousBlock, block);
        return true;
    }
    return false;
}

bool InplaceBuilder::setBiome(
    BlockSource&    source,
    BlockPos const& pos,
    Biome const&    biome
) {
    auto chunk = source.getChunkSource().getExistingChunk(ChunkPos{pos});
    if (!chunk
        || chunk->mLoadState->load(std::memory_order_relaxed)
               <= ChunkState::CheckingForReplacementData) {
        return false;
    }
    chunk->_setBiome(biome, ChunkBlockPos{pos, source.getMinHeight()}, false);
    return true;
}

} // namespace we
