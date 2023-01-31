//
// Created by OEOTYAN on 2023/01/28.
//
#pragma once
#include "PlayerData.h"
#include "eval/Eval.h"
#include "eval/CppEval.h"
#include <MC/Player.hpp>
#include <MC/Block.hpp>
#include <MC/VanillaBlocks.hpp>
#include <MC/BlockSource.hpp>
#include <MC/CommandUtils.hpp>
#include <MC/Level.hpp>
#include <MC/HitResult.hpp>
#include "region/Regions.h"
namespace worldedit {

    void PlayerData::clearHistory() {
        historyList.clear();
        historyList.resize(maxHistoryLength);
        historyFlag1 = 0;
        historyFlag2 = 0;
    }

    PlayerData::~PlayerData() {
        delete region;
        region = nullptr;
    }

    PlayerData::PlayerData() {
        historyList.resize(0);
    }

    PlayerData::PlayerData(::Player* player) {
        historyList.resize(maxHistoryLength);
        xuid = player->getXuid();
    }

    bool PlayerData::changeVicePos(BlockInstance blockInstance, bool output) {
        Player* player = Global<Level>->getPlayer(xuid);
        if (blockInstance == BlockInstance::Null) {
            if (output)
                player->sendText("worldedit.selection.empty.explain.secondary", "");
            return false;
        }
        if (region == nullptr) {
            region = new worldedit::CuboidRegion();
        }
        auto pos = blockInstance.getPosition();
        if (player->getDimensionId() != region->getDimensionID()) {
            region->selecting = false;
        }
        if (region->setVicePos(pos, player->getDimensionId())) {
            if (output)
                player->sendText("worldedit.selection." + region->getName() + ".explain.secondary",
                                          pos.toString(), region->size());
            vicePos = pos;
            vicePosTime = 0;
            vicePosDim = player->getDimensionId();
            return true;
        } else {
            if (output)
                player->sendText("worldedit.selection.failed.explain.secondary", "");
        }
        return false;
    }

    bool PlayerData::changeMainPos(BlockInstance blockInstance, bool output) {
        Player* player = Global<Level>->getPlayer(xuid);
        if (blockInstance == BlockInstance::Null) {
            if (output)
                player->sendText("worldedit.selection.empty.explain.primary", "");
            return false;
        }
        if (region == nullptr) {
            region = new worldedit::CuboidRegion();
        }
        auto pos = blockInstance.getPosition();
        if (player->getDimensionId() != region->getDimensionID()) {
            region->selecting = false;
        }
        if (region->setMainPos(pos, player->getDimensionId())) {
            if (output)
                player->sendText("worldedit.selection." + region->getName() + ".explain.primary",
                                          pos.toString(), region->size());
            mainPos = pos;
            mainPosTime = 0;
            mainPosDim = player->getDimensionId();
            if (region->needResetVice) {
                vicePosDim = -1;
            }
            return true;
        } else {
            if (output)
                player->sendText("worldedit.selection.failed.explain.primary", "");
        }
        return false;
    }

    void PlayerData::setVarByPlayer(
        std::unordered_map<::std::string, double>& variables) {
        Player* player = Global<Level>->getPlayer(xuid);
        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        variables["px"] = playerPos.x;
        variables["py"] = playerPos.y;
        variables["pz"] = playerPos.z;
        variables["pp"] = playerRot.x;
        variables["pt"] = playerRot.y;
    }

    bool PlayerData::setBlockForHistory(BlockSource* blockSource,
                                        const BlockPos& pos,
                                        Block* block,
                                        Block* exblock) {
        CommandUtils::clearBlockEntityContents(*blockSource, pos);
        blockSource->setExtraBlock(pos, *BedrockBlocks::mAir, 16 + updateArg);
        // if (updateExArg % 2 == 1) {
            blockSource->setBlock(pos, *block, updateArg, nullptr, nullptr);
        // }
        //  else {
        //     blockSource->setBlockNoUpdate(pos.x, pos.y, pos.z, *block);
        // }
        if (block != VanillaBlocks::mBubbleColumn) {
            if (exblock != BedrockBlocks::mAir) {
                blockSource->setExtraBlock(pos, *exblock, 16 + updateArg);
            }
        } else {
            blockSource->setExtraBlock(pos, *VanillaBlocks::mFlowingWater,
                                       16 + updateArg);
            // if (updateExArg % 2 == 1) {
                blockSource->setBlock(pos, *block, updateArg, nullptr, nullptr);
            // } 
            // else {
            //     blockSource->setBlockNoUpdate(pos.x, pos.y, pos.z, *block);
            // }
        }
        return true;
    }

    bool PlayerData::setBlockSimple(
        BlockSource* blockSource,
        class EvalFunctions& funcs,
        std::unordered_map<std::string, double> const& var,
        const BlockPos& pos,
        Block* block,
        Block* exblock) {
        if (gMask != "") {
            if (cpp_eval::eval<double>(gMask, var, funcs) <= 0.5) {
                return false;
            }
        }

        CommandUtils::clearBlockEntityContents(*blockSource, pos);
        blockSource->setExtraBlock(pos, *BedrockBlocks::mAir, 16 + updateArg);
        // if (updateExArg % 2 == 1) {
            blockSource->setBlock(pos, *block, updateArg, nullptr, nullptr);
        // } else {
        //     blockSource->setBlockNoUpdate(pos.x, pos.y, pos.z, *block);
        // }
        if (block != VanillaBlocks::mBubbleColumn) {
            if (exblock != BedrockBlocks::mAir) {
                blockSource->setExtraBlock(pos, *exblock, 16 + updateArg);
            }
        } else {
            blockSource->setExtraBlock(pos, *VanillaBlocks::mFlowingWater,
                                       16 + updateArg);
            // if (updateExArg % 2 == 1) {
                blockSource->setBlock(pos, *block, updateArg, nullptr, nullptr);
            // } else {
            //     blockSource->setBlockNoUpdate(pos.x, pos.y, pos.z, *block);
            // }
        }
        return true;
    }

    Clipboard* PlayerData::getNextHistory() {
        historyFlag2 += 1;
        historyFlag2 %= maxHistoryLength;
        historyFlag1 = historyFlag2;
        return &historyList[historyFlag2];
    }

    std::pair<Clipboard*, int> PlayerData::getUndoHistory() {
        std::pair<Clipboard*, int> result = {nullptr, -1};
        int maximum = (historyFlag1 + 1) % maxHistoryLength;
        int current = historyFlag2;
        if (maximum == current) {
            result.second = 0;
            return result;
        }
        result.first = &historyList[current];
        if (result.first->used == false) {
            result.second = 0;
            return result;
        }
        result.second = 1;
        current -= 1;
        if (current < 0) {
            current += maxHistoryLength;
        }
        historyFlag2 = current;
        return result;
    }

    std::pair<Clipboard*, int> PlayerData::getRedoHistory() {
        std::pair<Clipboard*, int> result = {nullptr, -1};
        int maximum = (historyFlag1 + 1) % maxHistoryLength;
        int current = historyFlag2 + 1;
        current %= maxHistoryLength;
        if (maximum == current) {
            result.second = 0;
            return result;
        }
        result.first = &historyList[current];
        if (result.first->used == false) {
            result.second = 0;
            return result;
        }
        result.second = 1;
        historyFlag2 = current;
        return result;
    }

    BlockInstance PlayerData::getBlockFromViewVector(FaceID& face,
                                                     bool includeLiquid,
                                                     bool solidOnly,
                                                     float maxDistance,
                                                     bool ignoreBorderBlocks,
                                                     bool fullOnly) {
        Player* player = Global<Level>->getPlayer(xuid);

        auto* bs = &player->getRegion();
        auto pos = player->getCameraPos();
        auto viewVec = player->getViewVector(1.0f);
        auto viewPos = pos + (viewVec * maxDistance);
        int maxDisManhattan = (int)((maxDistance + 1) * 2);
        HitResult result = const_cast<BlockSource*>(bs)->clip(pos, viewPos, includeLiquid, solidOnly, maxDisManhattan,
                                                              ignoreBorderBlocks, fullOnly);
        if (result.isHit() || (includeLiquid && result.isHitLiquid())) {
            BlockPos bpos{};
            if (includeLiquid && result.isHitLiquid()) {
                bpos = result.getLiquidPos();
                face = result.getLiquidFacing();
            } else {
                bpos = result.getBlockPos();
                face = result.getFacing();
            }
            // auto block = const_cast<Block*>(&bs.getBlock(bpos));
            return Level::getBlockInstance(bpos, player->getDimensionId());
        }
        return BlockInstance::Null;
    }

    BlockInstance PlayerData::getBlockFromViewVector(bool includeLiquid,
                                                     bool solidOnly,
                                                     float maxDistance,
                                                     bool ignoreBorderBlocks,
                                                     bool fullOnly) {
        FaceID face = FaceID::Unknown;
        return getBlockFromViewVector(face, includeLiquid, solidOnly, maxDistance, ignoreBorderBlocks, fullOnly);
    }

}  // namespace worldedit