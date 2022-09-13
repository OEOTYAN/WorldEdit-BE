//
// Created by OEOTYAN on 2022/06/10.
//

#include "ClipboardBrush.h"
#include "store/BlockPattern.hpp"
#include <MC/Player.hpp>
#include "WorldEdit.h"

namespace worldedit {
    ClipboardBrush::ClipboardBrush(unsigned short s, Clipboard& c, bool o, bool a)
        : Brush(s, nullptr), center(o), ignoreAir(a), clipboard(c) {}
    long long ClipboardBrush::set(Player* player, BlockInstance blockInstance) {
        auto& mod = worldedit::getMod();
        auto xuid = player->getXuid();
        BlockPos pbPos;
        if (center) {
            BoundingBox box(BlockPos::ZERO, clipboard.board);
            pbPos = blockInstance.getPosition() - box.getCenter() + clipboard.playerRelPos;
        } else {
            pbPos = blockInstance.getPosition();
        }
        auto dimID = player->getDimensionId();

        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        auto blockSource = &player->getRegion();
        BoundingBox box = clipboard.getBoundingBox() + pbPos;

        if (mod.maxHistoryLength > 0) {
            auto history = mod.getPlayerNextHistory(xuid);
            *history = Clipboard(box.max - box.min);
            history->playerRelPos.x = dimID;
            history->playerPos = box.min;
            box.forEachBlockInBox([&](const BlockPos& pos) {
                auto localPos = pos - box.min;
                auto blockInstance = blockSource->getBlockInstance(pos);
                history->storeBlock(blockInstance, localPos);
            });
        }

        INNERIZE_GMASK

        EvalFunctions f;
        f.setbs(blockSource);
        f.setbox(box);
        std::unordered_map<std::string, double> variables;
        setFunction(variables, playerPos, playerRot);
        long long i = 0;
        if (ignoreAir) {
            clipboard.forEachBlockInClipboard([&](const BlockPos& pos) {
                if (clipboard.getSet(pos).getBlock() == BedrockBlocks::mAir &&
                    clipboard.getSet(pos).getExBlock() == BedrockBlocks::mAir) {
                    return;
                }
                auto worldPos = clipboard.getPos(pos) + pbPos;

                setFunction(variables, f, box, playerPos, worldPos, box.toAABB().getCenter());
                gMaskLambda(f, variables, [&]() mutable {
                    clipboard.setBlocks(pos, worldPos, blockSource);
                    ++i;
                });
            });
        } else {
            clipboard.forEachBlockInClipboard([&](const BlockPos& pos) {
                auto worldPos = clipboard.getPos(pos) + pbPos;
                setFunction(variables, f, box, playerPos, worldPos, box.toAABB().getCenter());
                gMaskLambda(f, variables, [&]() mutable {
                    clipboard.setBlocks(pos, worldPos, blockSource);
                    ++i;
                });
            });
        }
        return i;
    }
}  // namespace worldedit