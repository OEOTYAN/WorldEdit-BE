#include "InplaceBuilder.h"
#include "utils/DirectAccessBlocks.h"
#include <mc/world/level/chunk/LevelChunk.h>

namespace we {

bool InplaceBuilder::setBlock(
    BlockSource&                source,
    BlockPos const&             pos,
    Block const&                block,
    std::shared_ptr<BlockActor> blockActor
) const {
    bool applied = false;
    {
        // for medium
        Block const* extra;
        if (auto medium = block.getBlockType().getRequiredMedium(); medium) [[unlikely]] {
            extra = Block::tryGetFromRegistry(*medium);
        } else {
            extra = &DirectAccessBlocks::Air();
        }
        applied |= source.setExtraBlock(pos, *extra, updateFlags);
    }
    applied |= source.setBlock(pos, block, updateFlags, blockActor, nullptr, nullptr);
    return applied;
}

bool InplaceBuilder::setExtraBlock(
    BlockSource&    source,
    BlockPos const& pos,
    Block const&    block
) const {
    return source.setExtraBlock(pos, block, updateFlags);
}

bool InplaceBuilder::setBiome(
    BlockSource&    source,
    BlockPos const& pos,
    Biome const&    biome
) const {
    auto* chunk = source.getChunkAt(pos);
    if (!chunk) return false;
    chunk->_setBiome(biome, ChunkBlockPos{pos, source.getMinHeight()}, false);
    return true;
}

} // namespace we
