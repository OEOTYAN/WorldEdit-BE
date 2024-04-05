//
// Created by OEOTYAN on 2023/01/28.
//
#pragma once
#include "PlayerData.h"
#include "WorldEdit.h"
#include "eval/Eval.h"
#include "eval/CppEval.h"
#include <mc/Player.hpp>
#include <mc/Block.hpp>
#include <mc/StaticVanillaBlocks.hpp>
#include <mc/BlockSource.hpp>
#include <mc/ChunkSource.hpp>
#include <mc/BlockActor.hpp>
#include <mc/LevelChunk.hpp>
#include <mc/SubChunk.hpp>
#include <mc/Dimension.hpp>
#include <mc/SubChunkPos.hpp>
#include <mc/CommandUtils.hpp>
#include <mc/Level.hpp>
#include "region/Regions.h"
namespace worldedit {

    void PlayerData::clearHistory() {
        historyList.clear();
        historyList.resize(maxHistoryLength);
        historyFlag1 = 0;
        historyFlag2 = 0;
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
                player->trSendText("worldedit.selection.empty.explain.secondary");
            return false;
        }
        if (region == nullptr) {
            region = Region::createRegion(RegionType::CUBOID);
        }
        auto pos = blockInstance.getPosition();
        if (player->getDimensionId() != region->getDimensionID()) {
            region->selecting = false;
        }
        if (region->setVicePos(pos, player->getDimensionId())) {
            if (output)
                player->trSendText("worldedit.selection." + region->getName() + ".explain.secondary", pos.toString(),
                                   region->size());
            vicePos = pos;
            vicePosTime = 0;
            vicePosDim = player->getDimensionId();
            return true;
        } else {
            if (output)
                player->trSendText("worldedit.selection.failed.explain.secondary");
        }
        return false;
    }

    bool PlayerData::changeMainPos(BlockInstance blockInstance, bool output) {
        Player* player = Global<Level>->getPlayer(xuid);
        if (blockInstance == BlockInstance::Null) {
            if (output)
                player->trSendText("worldedit.selection.empty.explain.primary");
            return false;
        }
        if (region == nullptr) {
            region = Region::createRegion(RegionType::CUBOID);
        }
        auto pos = blockInstance.getPosition();
        if (player->getDimensionId() != region->getDimensionID()) {
            region->selecting = false;
        }
        if (region->setMainPos(pos, player->getDimensionId())) {
            if (output)
                player->trSendText("worldedit.selection." + region->getName() + ".explain.primary", pos.toString(),
                                   region->size());
            mainPos = pos;
            mainPosTime = 0;
            mainPosDim = player->getDimensionId();
            if (region->needResetVice) {
                vicePosDim = -1;
            }
            return true;
        } else {
            if (output)
                player->trSendText("worldedit.selection.failed.explain.primary");
        }
        return false;
    }

    void PlayerData::setVarByPlayer(phmap::flat_hash_map<::std::string, double>& variables) {
        Player* player = Global<Level>->getPlayer(xuid);
        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        variables["px"] = playerPos.x;
        variables["py"] = playerPos.y;
        variables["pz"] = playerPos.z;
        variables["pp"] = playerRot.x;
        variables["pt"] = playerRot.y;
    }

    bool PlayerData::setBlockWithoutcheckGMask(BlockSource* blockSource,
                                               const BlockPos& pos,
                                               Block const* block,
                                               Block const* exblock,
                                               std::optional<int> const& biomeId) {
        bool res = false;

        auto& dim = blockSource->getDimension();
        ChunkBlockPos cpos(pos, dim.getMinHeight());
        SubChunkPos scpos(pos);
        LevelChunk* chunk = blockSource->getChunkAt(pos);
        if (chunk == nullptr || chunk->isReadOnly()) {
            return false;
        }

        BlockActor* be = chunk->getBlockEntity(cpos);
        if (be != nullptr) {
            if (be->getType() != block->getBlockEntityType()) {
                chunk->removeBlockEntity(pos);
            }
        }

        if (updateExArg % 2 == 1) {
            res |= blockSource->setExtraBlock(pos, *BedrockBlocks::mAir, updateArg);
            res |= blockSource->setBlock(pos, *block, updateArg, nullptr, nullptr);
            if (block != StaticVanillaBlocks::mBubbleColumn) {
                if (exblock != BedrockBlocks::mAir) {
                    res |= blockSource->setExtraBlock(pos, *exblock, updateArg);
                }
            } else {
                exblock = StaticVanillaBlocks::mFlowingWater;
                res |= blockSource->setExtraBlock(pos, *exblock, updateArg + 2);
                res |= blockSource->setBlock(pos, *block, updateArg + 2, nullptr, nullptr);
            }
        } else {
            auto* sc = chunk->getSubChunk(scpos.y);
            auto index = cpos.toLegacyIndex();
            if (&chunk->getBlock(cpos) != block) {
                res = true;
                sc->_setBlock(0, index, *block);
            }
            if (&chunk->getExtraBlock(cpos) != exblock) {
                res = true;
                sc->_setBlock(1, index, *exblock);
            }
        }
        if (res) {
            dim.forEachPlayer([&](Player& player) -> bool {
                player.sendUpdateBlockPacket(pos, *exblock, UpdateBlockFlags::BlockUpdateAll,
                                             UpdateBlockLayer::UpdateBlockLiquid);
                player.sendUpdateBlockPacket(pos, *block, UpdateBlockFlags::BlockUpdateAll,
                                             UpdateBlockLayer::UpdateBlockDefault);
                player.sendUpdateBlockPacket(pos, *exblock, UpdateBlockFlags::BlockUpdateAll,
                                             UpdateBlockLayer::UpdateBlockLiquid);
                return true;
            });
        }
        if (biomeId.has_value()) {
            chunk->_setBiome(*Biome::fromId(biomeId.value()), cpos, false);
        }
        return res;
    }

    bool PlayerData::setBlockSimple(BlockSource* blockSource,
                                    class EvalFunctions& funcs,
                                    phmap::flat_hash_map<std::string, double> const& var,
                                    const BlockPos& pos,
                                    Block const* block,
                                    Block const* exblock,
                                    std::optional<int> const& biomeId) {
        if (gMask != "") {
            if (cpp_eval::eval<double>(gMask, var, funcs) <= 0.5) {
                return false;
            }
        }
        return setBlockWithoutcheckGMask(blockSource, pos, block, exblock, biomeId);
    }

    class Clipboard& PlayerData::getNextHistory() {
        historyFlag2 += 1;
        historyFlag2 %= maxHistoryLength;
        historyFlag1 = historyFlag2;
        return historyList[historyFlag2];
    }

    class Clipboard& PlayerData::getCurrentHistory() {
        return historyList[historyFlag2];
    }

    std::optional<std::reference_wrapper<class Clipboard>> PlayerData::getUndoHistory() {
        int maximum = (historyFlag1 + 1) % maxHistoryLength;
        int current = historyFlag2;
        if (maximum == current) {
            return std::nullopt;
        }
        auto& res = historyList[current];
        if (res.used == false) {
            return std::nullopt;
        }
        current -= 1;
        if (current < 0) {
            current += maxHistoryLength;
        }
        historyFlag2 = current;
        return res;
    }

    std::optional<std::reference_wrapper<class Clipboard>> PlayerData::getRedoHistory() {
        int maximum = (historyFlag1 + 1) % maxHistoryLength;
        int current = historyFlag2 + 1;
        current %= maxHistoryLength;
        if (maximum == current) {
            return std::nullopt;
        }
        auto& res = historyList[current];
        if (res.used == false) {
            return std::nullopt;
        }
        historyFlag2 = current;
        return res;
    }
}  // namespace worldedit