#pragma once

#include "BlockNBTSet.hpp"
#include "Globals.h"
#include <mc/BlockInstance.hpp>
#include <mc/Level.hpp>
#include <mc/StructureSettings.hpp>

namespace we {

class Clipboard {
public:
    std::unique_ptr<CompoundTag>   entities = nullptr;
    BlockPos                       size;
    BlockPos                       playerRelPos;
    BlockPos                       playerPos;
    BlockPos                       board;
    Rotation                       rotation;
    Mirror                         mirror;
    Vec3                           rotationAngle;
    bool                           flipY = false;
    bool                           used  = false;
    long long                      vsize;
    std::vector<class BlockNBTSet> blockslist;
    Clipboard() = default;
    Clipboard(const Clipboard& other);
    Clipboard(Clipboard&&)            = default;
    Clipboard& operator=(Clipboard&&) = default;
    Clipboard(BlockPos const& sizes);
    long long    getIter(BlockPos const& pos);
    long long    getIterLoop(BlockPos const& pos);
    void         storeBlock(BlockInstance& blockInstance, BlockPos const& pos);
    BoundingBox  getBoundingBox();
    void         rotate(Vec3 angle);
    void         flip(enum class FACING facing);
    BlockPos     getPos(BlockPos const& pos);
    BlockNBTSet& getSet(BlockPos const& pos);
    BlockNBTSet& getSetLoop(BlockPos const& pos);
    bool         contains(BlockPos const& pos);
    bool         setBlocks(
                BlockPos const&                                  pos,
                BlockPos&                                        worldPos,
                BlockSource*                                     blockSource,
                class PlayerData&                                data,
                class EvalFunctions&                             funcs,
                phmap::flat_hash_map<std::string, double> const& var,
                bool                                             setBiome = false
            );
    void forEachBlockInClipboard(const std::function<void(BlockPos const&)>& todo);
};
} // namespace we