//
// Created by OEOTYAN on 2022/05/21.
//
#pragma once

#include "Globals.h"
#include <mc/Level.hpp>
#include <mc/BlockInstance.hpp>
#include <mc/StructureSettings.hpp>
#include "BlockNBTSet.hpp"

namespace worldedit {

    class Clipboard {
       public:
        std::unique_ptr<CompoundTag> entities = nullptr;
        BlockPos size;
        BlockPos playerRelPos;
        BlockPos playerPos;
        BlockPos board;
        Rotation rotation;
        Mirror mirror;
        Vec3 rotationAngle;
        bool flipY = false;
        bool used = false;
        long long vsize;
        std::vector<class BlockNBTSet> blockslist;
        explicit Clipboard() = default;
        explicit Clipboard(const Clipboard& other);
        explicit Clipboard(Clipboard&&) = default;
        Clipboard& operator=(Clipboard&&) = default;
        explicit Clipboard(const BlockPos& sizes);
        long long getIter(const BlockPos& pos);
        long long getIterLoop(const BlockPos& pos);
        void storeBlock(BlockInstance& blockInstance, const BlockPos& pos);
        BoundingBox getBoundingBox();
        void rotate(Vec3 angle);
        void flip(enum class FACING facing);
        BlockPos getPos(const BlockPos& pos);
        BlockNBTSet& getSet(const BlockPos& pos);
        BlockNBTSet& getSetLoop(const BlockPos& pos);
        bool contains(const BlockPos& pos);
        bool setBlocks(const BlockPos& pos,
                       BlockPos& worldPos,
                       BlockSource* blockSource,
                       class PlayerData& data,
                       class EvalFunctions& funcs,
                       phmap::flat_hash_map<std::string, double> const& var,
                       bool setBiome = false);
        void forEachBlockInClipboard(const std::function<void(const BlockPos&)>& todo);
    };
}  // namespace worldedit