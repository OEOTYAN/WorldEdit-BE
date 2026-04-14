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
    return applied;
}

BlockOperation BlockOperation::record(
    LocalContext&,
    BlockSource&    source,
    BlockPos const& pos,
    bool&           deferred
) const {
    BlockOperation result;
    auto           chunk = tryGetUsableChunk(source, pos);
    if (!chunk) {
        // chunk not loaded
        deferred = false;
        return result;
    }
    deferred = true;
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
    return result;
}

bool BiomeOperation::apply(
    LocalContext&   context,
    BlockSource&    source,
    BlockPos const& pos
) const {
    if (!biome) {
        return false;
    }
    return context.setBiome(source, pos, *biome);
}

BiomeOperation BiomeOperation::record(
    LocalContext&,
    BlockSource&    source,
    BlockPos const& pos,
    bool&           deferred
) const {
    BiomeOperation result;
    auto           chunk = tryGetUsableChunk(source, pos);
    if (!chunk) {
        deferred = false;
        return result;
    }
    deferred     = true;
    result.biome = &source.getBiome(pos);
    return result;
}
} // namespace we
