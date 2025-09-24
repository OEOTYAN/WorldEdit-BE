#include "InplaceBuilder.h"
#include <mc/world/level/chunk/LevelChunk.h>

namespace we {

bool InplaceBuilder::setBlock(
    BlockSource&                source,
    BlockPos const&             pos,
    Block const&                block,
    std::shared_ptr<BlockActor> blockActor
) const {
    return source.setBlock(pos, block, updateFlags, blockActor, nullptr, nullptr);
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
