//
// Created by OEOTYAN on 2022/05/21.
//
#pragma once
#ifndef WORLDEDIT_CLIPBOARD_H
#define WORLDEDIT_CLIPBOARD_H

#include "pch.h"
// #include <EventAPI.h>
// #include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
// #include <MC/Block.hpp>
// #include <MC/BlockActor.hpp>
// #include <MC/BlockSource.hpp>
// #include <MC/CompoundTag.hpp>
#include <MC/StructureSettings.hpp>
// #include <MC/VanillaBlockStateTransformUtils.hpp>
// #include <MC/Actor.hpp>
// #include <MC/Player.hpp>
// #include <MC/ServerPlayer.hpp>
// #include <MC/Dimension.hpp>
// #include <MC/ItemStack.hpp>
// #include "string/StringTool.h"
// #include "particle/Graphics.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
// #include <DynamicCommandAPI.h>
// #include "store/BlockNBTSet.hpp"
// #include "WorldEdit.h"

#ifndef __M__PI__
#define __M__PI__ 3.141592653589793238462643383279
#endif

namespace worldedit {

    class Clipboard {
       public:
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
        explicit Clipboard()                 = default;
        explicit Clipboard(const Clipboard&) = default;
        explicit Clipboard(const BlockPos& sizes)
            : size(sizes + 1),
              board(sizes),
              rotation(Rotation::None_14),
              mirror(Mirror::None_15),
              rotationAngle({0, 0, 0}) {
            used  = true;
            vsize = size.x * size.y * size.z;
            blockslist.clear();
            try {
                blockslist.resize(vsize);
            } catch (std::bad_alloc) {
                Level::broadcastText("Out of memory", TextType::RAW);
                return;
            }
        }
        long long   getIter(const BlockPos& pos);
        long long   getIter2(const BlockPos& pos);
        void        storeBlock(BlockInstance& blockInstance, const BlockPos& pos);
        BoundingBox getBoundingBox();
        void        rotate(Vec3 angle);
        void        flip(enum class FACING facing);
        BlockPos    getPos(const BlockPos& pos);
        BlockNBTSet& getSet(const BlockPos& pos);
        BlockNBTSet& getSet2(const BlockPos& pos);
        bool        contains(const BlockPos& pos);
        void        setBlocks(const BlockPos& pos, BlockPos& worldPos, BlockSource* blockSource);
        void        forEachBlockInClipboard(const std::function<void(const BlockPos&)>& todo);
    };
}  // namespace worldedit

#endif  // WORLDEDIT_CLIPBOARD_H