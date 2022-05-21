//
// Created by OEOTYAN on 2022/05/21.
//
#pragma once
#ifndef WORLDEDIT_CLIPBOARD_H
#define WORLDEDIT_CLIPBOARD_H

#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockActor.hpp>
#include <MC/BlockSource.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/Dimension.hpp>
#include <MC/ItemStack.hpp>
#include "string/StringTool.h"
#include <LLAPI.h>
#include <ServerAPI.h>
#include <EventAPI.h>
#include <DynamicCommandAPI.h>
#include "WorldEdit.h"
#include "store/BlockNBTSet.hpp"

namespace worldedit {

    class Clipboard {
       public:
        BlockPos size;
        BlockPos playerRelPos;
        BlockPos board;
        long long vsize;
        std::vector<blockNBTSet> blockslist;
        Clipboard() = default;
        Clipboard(const BlockPos& sizes)
            : size(sizes + BlockPos(1, 1, 1)), board(sizes) {
            vsize = size.x * size.y * size.z;
            blockslist.clear();
            blockslist.resize(vsize);
        }
        long long getIter(const BlockPos& pos) {
            return (pos.y + size.y * pos.z) * size.x + pos.x;
        }
        void storeBlock(BlockInstance& blockInstance, const BlockPos& pos) {
            if (pos.containedWithin(BlockPos(0, 0, 0), board)) {
                blockslist[getIter(pos)] = blockNBTSet(blockInstance);
            }
        }
        blockNBTSet getSet(const BlockPos& pos) {
            return blockslist[getIter(pos)];
        }
        bool contains(const BlockPos& pos) {
            return blockslist[getIter(pos)].hasBlock;
        }

        void forEachBlockInClipboard(
            const std::function<void(const BlockPos&)>& todo) {
            for (int x = 0; x < size.x; x++)
                for (int y = 0; y < size.y; y++)
                    for (int z = 0; z < size.z; z++) {
                        if (contains({x, y, z})) {
                            todo({x, y, z});
                        }
                    }
        }
    };

}  // namespace worldedit

#endif  // WORLDEDIT_CLIPBOARD_H