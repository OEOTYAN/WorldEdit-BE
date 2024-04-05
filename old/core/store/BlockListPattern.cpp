//
// Created by OEOTYAN on 2023/02/02.
//

#include "BlockNBTSet.hpp"
#include "mc/BlockPalette.hpp"
#include <mc/LevelChunk.hpp>
#include "mc/Level.hpp"
#include "utils/StringHelper.h"
#include <mc/Block.hpp>
#include <mc/BedrockBlocks.hpp>
#include <mc/BlockActor.hpp>
#include "WorldEdit.h"
#include "mc/ItemStack.hpp"
#include "mc/BlockPalette.hpp"
#include "mc/StaticVanillaBlocks.hpp"
#include "mc/Player.hpp"
#include "utils/RNG.h"
#include "BlockListPattern.h"

namespace worldedit {
    double Percents::getPercents(const phmap::flat_hash_map<::std::string, double>& variables, EvalFunctions& funcs) {
        if (std::holds_alternative<double>(val)) {
            return std::get<double>(val);
        }
        return cpp_eval::eval<double>(std::get<std::string>(val), variables, funcs);
    }
    Block const* RawBlock::getBlock(const phmap::flat_hash_map<::std::string, double>& variables, EvalFunctions& funcs) {
        switch (block.index()) {
            default:
                return BedrockBlocks::mAir;
            case 0:
                return std::get<Block const*>(block);
            case 1:
                return Block::create(static_cast<unsigned int>(
                    round(cpp_eval::eval<double>(std::get<std::string>(block), variables, funcs))));
            case 2:
                std::string blockName;
                auto& blockIds = std::get<blockid_t>(block);
                if (std::holds_alternative<std::string>(blockIds.first)) {
                    blockName = getBlockName(static_cast<int>(
                        round(cpp_eval::eval<double>(std::get<std::string>(blockIds.first), variables, funcs))));
                } else {
                    blockName = std::get<BlockNameType>(blockIds.first).val;
                }

                int mData = 0;
                if (blockIds.second.has_value()) {
                    auto& dataVal = blockIds.second.value();
                    mData = std::holds_alternative<int>(dataVal)
                                ? std::get<int>(dataVal)
                                : (static_cast<int>(
                                      round(cpp_eval::eval<double>(std::get<std::string>(dataVal), variables, funcs))));
                }
                return Block::create(blockName, mData);
        }
    }

    RawBlock* BlockListPattern::getRawBlock(const phmap::flat_hash_map<::std::string, double>& variables,
                                            EvalFunctions& funcs) {
        std::vector<double> weights;
        double total = 0;
        weights.resize(blockNum);
        for (int i = 0; i < blockNum; ++i) {
            weights[i] = std::max(percents[i].getPercents(variables, funcs), 0.0);
            total += weights[i];
        }
        if (total < 1e-32) {
            return nullptr;
        }
        double random = RNG::rand<double>() * total;
        double sum = 0;
        for (int i = 0; i < blockNum - 1; ++i) {
            sum += weights[i];
            if (random <= sum) {
                return &rawBlocks[i];
            }
        }
        return &rawBlocks[blockNum - 1];
    }

    RawBlock::RawBlock() {
        exBlock = const_cast<class Block const*>(BedrockBlocks::mAir);
        block = std::pair{BlockNameType(""), std::nullopt};
    }

    Block const* BlockListPattern::getBlock(const phmap::flat_hash_map<::std::string, double>& variables,
                                      EvalFunctions& funcs) {
        auto* rawBlock = getRawBlock(variables, funcs);
        if (rawBlock == nullptr) {
            return nullptr;
        }
        return rawBlock->getBlock(variables, funcs);
    }

    bool BlockListPattern::setBlock(const phmap::flat_hash_map<::std::string, double>& variables,
                                    EvalFunctions& funcs,
                                    BlockSource* blockSource,
                                    const BlockPos& pos) {
        auto* rawBlock = getRawBlock(variables, funcs);
        if (rawBlock == nullptr) {
            return false;
        }
        auto* block = rawBlock->getBlock(variables, funcs);
        bool res = playerData->setBlockSimple(blockSource, funcs, variables, pos, block, rawBlock->exBlock);

        if (block->hasBlockEntity() && rawBlock->blockEntity != nullptr) {
            auto be = blockSource->getBlockEntity(pos);
            if (be != nullptr) {
                return be->setNbt(rawBlock->blockEntity.get());
            } else {
                LevelChunk* chunk = blockSource->getChunkAt(pos);
                auto b = BlockActor::create(rawBlock->blockEntity.get());
                if (b != nullptr) {
                    b->moveTo(pos);
                    chunk->_placeBlockEntity(b);
                }
            }
        }
        return res;
    }

    BlockListPattern::BlockListPattern(std::string_view str_v, std::string_view xuid) : Pattern(xuid) {
        type = Pattern::PatternType::BLOCKLIST;

        std::string str = asString(str_v);

        auto strSize = str.size();
        playerData = &getPlayersData(xuid);
        if (str == "#hand") {
            type = Pattern::PatternType::HAND;
            blockNum = 1;
            percents.resize(blockNum);
            rawBlocks.resize(blockNum);
            Player* player = Global<Level>->getPlayer(asString(xuid));
            if (player != nullptr) {
                auto mBlock = player->getHandSlot()->getBlock();
                if (mBlock != nullptr) {
                    rawBlocks[0].block = mBlock;
                }
            }
            return;
        }
        std::vector<std::string> raw;
        raw.clear();
        string form = "";
        size_t i = 0;
        while (i < strSize) {
            if (str[i] == '-' || isdigit(str[i])) {
                auto head = i;
                ++i;
                while (i < strSize && (str[i] == '.' || isdigit(str[i]))) {
                    ++i;
                }
                form += string("num") + str[i];
                raw.push_back(str.substr(head, i - head));
            } else if (str[i] == '\'') {
                ++i;
                auto head = i;
                while (i < strSize && (str[i] != '\'')) {
                    ++i;
                }
                raw.push_back(str.substr(head, i - head));
                ++i;
                form += string("funciton") + str[i];
            } else if (isalpha(str[i])) {
                if (str[i] == 'r' && str[i + 1] == 't' && str[i + 2] == '\'') {
                    i += 3;
                    auto head = i;
                    while (i < strSize && (str[i] != '\'')) {
                        ++i;
                    }
                    raw.push_back(str.substr(head, i - head));
                    ++i;
                    form += string("rtfunciton") + str[i];
                } else {
                    auto head = i;
                    ++i;
                    while (i < strSize && (isalpha(str[i]) || str[i] == '_' || isdigit(str[i]) ||
                                           (str[i] == ':' && i + 1 < strSize && isalpha(str[i + 1])))) {
                        ++i;
                    }
                    int kb = 0;
                    if (str[i] == '[') {
                        kb = 1;
                        ++i;
                        while (kb > 0) {
                            if (str[i] == '[') {
                                ++kb;
                            } else if (str[i] == ']') {
                                --kb;
                            }
                            ++i;
                        }
                    }
                    form += string("block") + str[i];
                    raw.push_back(str.substr(head, i - head));
                }
            } else if (str[i] == '{') {
                auto head = i;
                int bracket = -1;
                ++i;
                while (i < strSize && bracket < 0) {
                    if (str[i] == '{') {
                        bracket--;
                    } else if (str[i] == '}') {
                        bracket++;
                    }
                    ++i;
                }
                form += string("SNBT") + str[i];
                raw.push_back(str.substr(head, i - head));
            }
            ++i;
        }
        auto blockList = SplitStrWithPattern(form, ",");
        blockNum = blockList.size();
        // for (auto& x : blockList) {
        //     std::cout << x << std::endl;
        // }
        percents.resize(blockNum);
        rawBlocks.resize(blockNum);
        int rawPtr = 0;
        for (int iter = 0; iter < blockNum; iter++) {
            auto& blockIter = blockList[iter];
            auto& rawBlockIter = rawBlocks[iter];
            if (blockIter.find("%") != std::string::npos) {
                if (blockIter.find("num%") != std::string::npos) {
                    percents[iter].val = std::stod(raw[rawPtr]);
                } else {
                    percents[iter].val = raw[rawPtr];
                }
                ++rawPtr;
            } else {
                blockIter = "%" + blockIter;
            }
            if (blockIter.find("%num") != std::string::npos) {
                std::get<RawBlock::blockid_t>(rawBlockIter.block).first =
                    BlockNameType(getBlockName(std::stoi(raw[rawPtr])));
                ++rawPtr;
            } else if (blockIter.find("%block") != std::string::npos) {
                std::string tmpName = raw[rawPtr];
                if (tmpName[0] == 'r' && tmpName[1] == 't' && isdigit(tmpName[2])) {
                    rawBlockIter.block = Block::create(static_cast<unsigned int>(std::stoi(tmpName.substr(2))));
                    ++rawPtr;
                    continue;
                }
                if (tmpName.find("minecraft:") == std::string::npos) {
                    tmpName = "minecraft:" + tmpName;
                }
                std::get<RawBlock::blockid_t>(rawBlockIter.block).first = BlockNameType(tmpName);
                ++rawPtr;
            } else if (blockIter.find("%rtfunciton") != std::string::npos) {
                rawBlockIter.block = raw[rawPtr];
                ++rawPtr;
                continue;
            } else if (blockIter.find("%funciton") != std::string::npos) {
                std::get<RawBlock::blockid_t>(rawBlockIter.block).first = raw[rawPtr];
                ++rawPtr;
            } else if (blockIter.find("%SNBT") != std::string::npos) {
                std::string tmpSNBT = raw[rawPtr];
                if (tmpSNBT[0] == '{' && tmpSNBT[1] == '{') {
                    size_t i2 = 1;
                    std::vector<std::string> tmpSNBTs;
                    tmpSNBTs.clear();
                    while (i2 < tmpSNBT.size() - 1) {
                        int bracket2 = -1;
                        if (tmpSNBT[i2] == '{') {
                            auto head2 = i2;
                            i2++;
                            while (i2 < tmpSNBT.size() && bracket2 < 0) {
                                if (tmpSNBT[i2] == '{') {
                                    bracket2--;
                                } else if (tmpSNBT[i2] == '}') {
                                    bracket2++;
                                }
                                i2++;
                            }
                            tmpSNBTs.push_back(tmpSNBT.substr(head2, i2 - head2));
                        }
                        i2++;
                    }
                    for (auto& ssss : tmpSNBTs) {
                        logger().info("{}", ssss);
                    }
                    rawBlockIter.block = Block::create(CompoundTag::fromSNBT(tmpSNBTs[0]).get());
                    if (tmpSNBTs.size() > 1) {
                        rawBlockIter.exBlock = Block::create(CompoundTag::fromSNBT(tmpSNBTs[1]).get());
                    }
                    if (tmpSNBTs.size() > 2) {
                        rawBlockIter.blockEntity = CompoundTag::fromSNBT(tmpSNBTs[2]);
                    }
                } else {
                    rawBlockIter.block = Block::create(CompoundTag::fromSNBT(tmpSNBT).get());
                }
                ++rawPtr;
                continue;
            }
            if (blockIter.find(":num") != std::string::npos) {
                std::get<RawBlock::blockid_t>(rawBlockIter.block).second = std::stoi(raw[rawPtr]);
                ++rawPtr;
            } else if (blockIter.find(":funciton") != std::string::npos) {
                std::get<RawBlock::blockid_t>(rawBlockIter.block).second = raw[rawPtr];
                ++rawPtr;
            }
            if (std::holds_alternative<RawBlock::blockid_t>(rawBlockIter.block) &&
                std::holds_alternative<BlockNameType>(std::get<RawBlock::blockid_t>(rawBlockIter.block).first) &&
                !(std::get<RawBlock::blockid_t>(rawBlockIter.block).second.has_value() &&
                  std::holds_alternative<std::string>(
                      std::get<RawBlock::blockid_t>(rawBlockIter.block).second.value()))) {
                auto& blockName = std::get<BlockNameType>(std::get<RawBlock::blockid_t>(rawBlockIter.block).first).val;
                if (isBEBlock(blockName)) {
                    rawBlockIter.block = Block::create(
                        blockName, std::get<int>(std::get<RawBlock::blockid_t>(rawBlockIter.block).second.value_or(0)));
                } else if (isJEBlock(blockName)) {
                    rawBlockIter.block = getJavaBlockMap()[blockName];
                    if (blockName.find("waterlogged=true") != std::string::npos) {
                        rawBlockIter.exBlock = StaticVanillaBlocks::mWater;
                    }
                } else {
                    rawBlockIter.block = BedrockBlocks::mAir;
                }
            }
        }
    }

    bool BlockListPattern::hasBlock(Block const* block) {
        for (auto& rawBlock : rawBlocks) {
            if (std::holds_alternative<Block const*>(rawBlock.block)) {
                if (std::get<Block const*>(rawBlock.block) == block) {
                    return true;
                }
            }
        }
        return false;
    }
}  // namespace worldedit