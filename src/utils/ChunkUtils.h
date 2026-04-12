#pragma once

#include <atomic>
#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/ChunkPos.h>
#include <mc/world/level/chunk/ChunkSource.h>
#include <mc/world/level/chunk/LevelChunk.h>

namespace we {

[[nodiscard]] inline std::shared_ptr<LevelChunk>
tryGetUsableChunk(BlockSource& source, BlockPos const& pos) {
    auto chunk = source.getChunkSource().getExistingChunk(ChunkPos{pos});
    if (!chunk
        || chunk->mLoadState->load(std::memory_order_relaxed)
               <= ChunkState::CheckingForReplacementData) {
        return nullptr;
    }
    return chunk;
}

} // namespace we