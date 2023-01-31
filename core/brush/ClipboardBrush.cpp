//
// Created by OEOTYAN on 2022/06/10.
//

#include "ClipboardBrush.h"
#include "store/BlockPattern.hpp"
#include "store/BlockNBTSet.hpp"
#include <MC/Player.hpp>
#include "WorldEdit.h"

namespace worldedit {
    ClipboardBrush::ClipboardBrush(unsigned short s,
                                   Clipboard& c,
                                   bool o,
                                   bool a)
        : Brush(s, nullptr), center(o), ignoreAir(a), clipboard(c) {}
    long long ClipboardBrush::set(Player* player, BlockInstance blockInstance) {
        
        auto xuid = player->getXuid();
        auto& playerData = getPlayersData(xuid);
        BlockPos pbPos;
        if (center) {
            BoundingBox box(BlockPos::ZERO, clipboard.board);
            pbPos = blockInstance.getPosition() - box.getCenter() +
                    clipboard.playerRelPos;
        } else {
            pbPos = blockInstance.getPosition();
        }
        auto dimID = player->getDimensionId();

        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        auto blockSource = &player->getRegion();
        BoundingBox box = clipboard.getBoundingBox() + pbPos;

        if (playerData.maxHistoryLength > 0) {
            auto history = playerData.getNextHistory();
            *history = Clipboard(box.max - box.min);
            history->playerRelPos.x = dimID;
            history->playerPos = box.min;
            box.forEachBlockInBox([&](const BlockPos& pos) {
                auto localPos = pos - box.min;
                auto blockInstance = Level::getBlockInstance(pos,player->getDimensionId());
                history->storeBlock(blockInstance, localPos);
            });
        }

        EvalFunctions f;
        f.setbs(blockSource);
        f.setbox(box);
        std::unordered_map<std::string, double> variables;
        playerData.setVarByPlayer(variables);
        long long i = 0;
        if (ignoreAir) {
            clipboard.forEachBlockInClipboard([&](const BlockPos& pos) {
                if (clipboard.getSet(pos).getBlock() == BedrockBlocks::mAir &&
                    clipboard.getSet(pos).getExBlock() == BedrockBlocks::mAir) {
                    return;
                }
                auto worldPos = clipboard.getPos(pos) + pbPos;

                setFunction(variables, f, box, playerPos, worldPos,
                            box.toAABB().getCenter());
                maskFunc(f, variables, [&]() mutable {
                    i += playerData.clipboard.setBlocks(
                        pos, worldPos, blockSource, playerData, f, variables);
                });
            });
        } else {
            clipboard.forEachBlockInClipboard([&](const BlockPos& pos) {
                auto worldPos = clipboard.getPos(pos) + pbPos;
                setFunction(variables, f, box, playerPos, worldPos,
                            box.toAABB().getCenter());
                maskFunc(f, variables, [&]() mutable {
                    i += playerData.clipboard.setBlocks(
                        pos, worldPos, blockSource, playerData, f, variables);
                });
            });
        }
        return i;
    }
}  // namespace worldedit