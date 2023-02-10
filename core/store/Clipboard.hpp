//
// Created by OEOTYAN on 2022/05/21.
//
#pragma once

#include "Global.h"
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/StructureSettings.hpp>
#include "I18nAPI.h"
#include "BlockNBTSet.hpp"

namespace worldedit {

    class Clipboard {
       public:
        std::string entityStr;
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
        explicit Clipboard(const Clipboard&) = default;
        explicit Clipboard(const BlockPos& sizes)
            : size(sizes + 1),
              board(sizes),
              rotation(Rotation::None_14),
              mirror(Mirror::None_15),
              rotationAngle({0, 0, 0}) {
            used = true;
            vsize = size.x * size.y * size.z;
            blockslist.clear();
            try {
                blockslist.resize(vsize);
            } catch (...) {
                Level::broadcastText(tr("worldedit.memory.out"), TextType::RAW);
                return;
            }
        }
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
                       std::unordered_map<std::string, double> const& var);
        void forEachBlockInClipboard(const std::function<void(const BlockPos&)>& todo);
    };
}  // namespace worldedit