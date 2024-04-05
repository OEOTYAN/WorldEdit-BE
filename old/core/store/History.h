//
// Created by OEOTYAN on 2023/03/26.
//

#pragma once
#include "Globals.h"
#include "mc/CompoundTag.hpp"
#include "mc/BlockActor.hpp"
#include "mc/Biome.hpp"

namespace worldedit {

    class ChunkHistory {
       public:
        phmap::flat_hash_set<ActorUniqueID> mAddedEntities;
        phmap::flat_hash_map<ActorUniqueID, std::unique_ptr<CompoundTag>> mRemovedEntities;
        phmap::flat_hash_map<ChunkBlockPos, std::shared_ptr<BlockActor>> mBlockEntities;
        phmap::flat_hash_map<SubChunkPos, std::unique_ptr<std::array<Biome*, 4096>>> mBiomes;
        phmap::flat_hash_map<SubChunkPos, std::unique_ptr<std::array<std::array<Block const*, 4096>, 2>>> mBlocks;
    };
    class History {
       public:
        class BlockSource* mBlockSource;
        short minHeight;
        bool needEntities = false;
        bool needBiomes = false;
        bool used = false;
        phmap::flat_hash_set<BlockPos> storedPos;
        phmap::flat_hash_map<ChunkPos, ChunkHistory> mChunks;
    };
}  // namespace worldedit
