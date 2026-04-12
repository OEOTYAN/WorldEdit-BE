#include "Operation.h"
#include "data/LocalContext.h"
#include "utils/ChunkUtils.h"

namespace we {
bool BlockOperation::apply(
    LocalContext&   context,
    BlockSource&    source,
    BlockPos const& pos
) const {
    bool applied = false;
    if (block) {
        applied |= context.setBlock(source, pos, *block, blockActor);
    }
    if (extraBlock) {
        applied |= context.setExtraBlock(source, pos, *extraBlock);
    }
    if (biome) {
        applied |= context.setBiome(source, pos, *biome);
    }
    return applied;
}

BlockOperation BlockOperation::record(
    LocalContext&,
    BlockSource&    source,
    BlockPos const& pos,
    bool&           valid
) const {
    BlockOperation result;
    if (pos.y < source.getMinHeight() || source.getMaxHeight() <= pos.y) {
        // out of height range
        valid = false;
        return result;
    }
    auto chunk = tryGetUsableChunk(source, pos);
    if (!chunk) {
        // chunk not loaded
        valid = false;
        return result;
    }
    valid = true;
    ChunkBlockPos chunkBlockPos{pos, source.getMinHeight()};
    if (block) {
        result.block      = &chunk->getBlock(chunkBlockPos);
        result.extraBlock = &chunk->getExtraBlock(chunkBlockPos);
        if (auto iter = chunk->mBlockEntities->mMap->find(chunkBlockPos);
            iter != chunk->mBlockEntities->mMap->end()) {
            CompoundTag nbt;
            if (iter->second->save(nbt, SaveContext{})) {
                result.blockActor = BlockActor::create(nbt, pos);
            }
        }
    }
    if (biome) {
        result.biome = &source.getBiome(pos);
    }
    return result;
}
} // namespace we
