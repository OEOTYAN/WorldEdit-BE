//
// Created by OEOTYAN on 2022/06/10.
//
#include "BlockPattern.hpp"
#include "BlockNBTSet.hpp"
#include "MC/BlockPalette.hpp"
#include "MC/Level.hpp"
#include "Utils/StringHelper.h"
#include <MC/Block.hpp>
#include <MC/BedrockBlocks.hpp>
#include <MC/BlockActor.hpp>
#include "WorldEdit.h"
#include "MC/ItemStack.hpp"
#include "MC/BlockPalette.hpp"
#include "MC/VanillaBlocks.hpp"
#include "MC/Player.hpp"
#include "region/Region.h"
#include "Utils/RNG.h"

namespace worldedit {

    double Percents::getPercents(const std::unordered_map<::std::string, double>& variables, EvalFunctions& funcs) {
        if (isNum) {
            return value;
        }
        return cpp_eval::eval<double>(function, variables, funcs);
    }
    Block* RawBlock::getBlock(const std::unordered_map<::std::string, double>& variables, EvalFunctions& funcs) {
        if (constBlock) {
            return block;
        }
        if (blockId == -2140000001) {
            return const_cast<Block*>(&Global<Level>->getBlockPalette().getBlock(
                static_cast<unsigned int>(round(cpp_eval::eval<double>(blockIdfunc, variables, funcs)))));
        }
        int mId = blockId;
        std::string blockName = "minecraft:air";
        if (mId == -2140000002) {
                blockName = blockIdfunc;
        } else {
            if (mId == INT_MIN) {
                mId = static_cast<int>(round(cpp_eval::eval<double>(blockIdfunc, variables, funcs)));
            } else if (mId == -2140000000) {
                mId = 0;
            }
            blockName = getBlockName(mId);
        }

        int mData = blockData;
        if (mData == INT_MIN) {
            mData = static_cast<int>(round(cpp_eval::eval<double>(blockDatafunc, variables, funcs)));
        } else if (mData == -2140000000) {
            mData = 0;
        }
        return Block::create(blockName, mData);
    }

    RawBlock* BlockPattern::getRawBlock(const std::unordered_map<::std::string, double>& variables,
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
        block = const_cast<class Block*>(BedrockBlocks::mAir);
        exBlock = const_cast<class Block*>(BedrockBlocks::mAir);
    }

    Block* BlockPattern::getBlock(const std::unordered_map<::std::string, double>& variables, EvalFunctions& funcs) {
        return getRawBlock(variables, funcs)->getBlock(variables, funcs);
    }

    bool BlockPattern::setBlock(const std::unordered_map<::std::string, double>& variables,
                                EvalFunctions& funcs,
                                BlockSource* blockSource,
                                const BlockPos& pos) {
        if (clipboard != nullptr) {
            return clipboard->getSetLoop(pos - bias).setBlock(pos, blockSource, *playerData, funcs, variables);
        }
        auto* rawBlock = getRawBlock(variables, funcs);
        if (rawBlock == nullptr) {
            return false;
        }
        auto* block = rawBlock->getBlock(variables, funcs);
        playerData->setBlockSimple(blockSource, funcs, variables, pos, block, rawBlock->exBlock);

        if (rawBlock->hasBE && block->hasBlockEntity()) {
            auto be = blockSource->getBlockEntity(pos);
            if (be != nullptr && rawBlock->blockEntity != "") {
                be->setNbt(CompoundTag::fromBinaryNBT(rawBlock->blockEntity).get());
            }
        }
        return true;
    }

    BlockPattern::BlockPattern(std::string str, std::string xuid, Region* region) {
        auto strSize = str.size();

        playerData = &getPlayersData(xuid);
        if (str == "#hand") {
            blockNum = 1;
            percents.resize(blockNum);
            rawBlocks.resize(blockNum);
            Player* player = Global<Level>->getPlayer(xuid);
            if (player != nullptr) {
                auto mBlock = player->getHandSlot()->getBlock();
                if (mBlock != nullptr) {
                    rawBlocks[0].block = const_cast<Block*>(mBlock);
                }
            }
            return;

        } else if (str.find("#clipboard") != std::string::npos) {
            if (!playerData->clipboard.used) {
                return;
            }
            clipboard = &playerData->clipboard;
            if (region != nullptr) {
                if (str.find("@c") != std::string::npos) {
                    bias = region->getCenter().toBlockPos();
                } else {
                    bias = region->getBoundBox().min;
                }
            } else {
                bias = BlockPos(0, 0, 0);
            }
            size_t mi = 0;
            std::vector<int> poslist;
            poslist.clear();
            while (mi < strSize) {
                if (str[mi] == '-' || isdigit(str[mi])) {
                    auto head7 = mi;
                    mi++;
                    while (mi < strSize && (isdigit(str[mi]))) {
                        mi++;
                    }
                    poslist.push_back(std::stoi(str.substr(head7, mi - head7)));
                }
                mi++;
            }
            auto poslistSize = poslist.size();
            if (poslistSize > 0) {
                bias.x -= poslist[0];
            }
            if (poslistSize > 1) {
                bias.y -= poslist[1];
            }
            if (poslistSize > 2) {
                bias.z -= poslist[2];
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
                    percents[iter].value = std::stod(raw[rawPtr]);
                } else {
                    percents[iter].isNum = false;
                    percents[iter].function = raw[rawPtr];
                }
                ++rawPtr;
            } else {
                blockIter = "%" + blockIter;
            }
            if (blockIter.find("%num") != std::string::npos) {
                rawBlockIter.blockId = -2140000002;
                rawBlockIter.blockIdfunc = getBlockName(std::stoi(raw[rawPtr]));
                ++rawPtr;
            } else if (blockIter.find("%block") != std::string::npos) {
                std::string tmpName = raw[rawPtr];
                if (tmpName[0] == 'r' && tmpName[1] == 't' && isdigit(tmpName[2])) {
                    rawBlockIter.block = const_cast<Block*>(&Global<Level>->getBlockPalette().getBlock(
                        static_cast<unsigned int>(std::stoi(tmpName.substr(2)))));
                    ++rawPtr;
                    continue;
                }
                if (tmpName.find("minecraft:") == std::string::npos) {
                    tmpName = "minecraft:" + tmpName;
                }
                rawBlockIter.blockId = -2140000002;
                rawBlockIter.blockIdfunc = tmpName;
                ++rawPtr;
            } else if (blockIter.find("%rtfunciton") != std::string::npos) {
                rawBlockIter.blockId = -2140000001;
                rawBlockIter.constBlock = false;
                rawBlockIter.blockIdfunc = raw[rawPtr];
                ++rawPtr;
                continue;
            } else if (blockIter.find("%funciton") != std::string::npos) {
                rawBlockIter.blockId = INT_MIN;
                rawBlockIter.constBlock = false;
                rawBlockIter.blockIdfunc = raw[rawPtr];
                ++rawPtr;
            } else if (blockIter.find("%SNBT") != std::string::npos) {
                rawBlockIter.blockId = INT_MIN;
                rawBlockIter.blockData = INT_MIN;
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
                    rawBlockIter.block = Block::create(CompoundTag::fromSNBT(tmpSNBTs[0]).get());
                    if (tmpSNBTs.size() > 1) {
                        rawBlockIter.exBlock = Block::create(CompoundTag::fromSNBT(tmpSNBTs[1]).get());
                    }
                    if (tmpSNBTs.size() > 2) {
                        rawBlockIter.blockEntity = CompoundTag::fromSNBT(tmpSNBTs[2])->toBinaryNBT();
                    }
                } else {
                    rawBlockIter.block = Block::create(CompoundTag::fromSNBT(tmpSNBT).get());
                }
                ++rawPtr;
                continue;
            }
            if (blockIter.find(":num") != std::string::npos) {
                rawBlockIter.blockData = std::stoi(raw[rawPtr]);
                ++rawPtr;
            } else if (blockIter.find(":funciton") != std::string::npos) {
                rawBlockIter.blockData = INT_MIN;
                rawBlockIter.constBlock = false;
                rawBlockIter.blockDatafunc = raw[rawPtr];
                ++rawPtr;
            }
            if (rawBlockIter.constBlock) {
                if (isBEBlock(rawBlockIter.blockIdfunc)) {
                    rawBlockIter.block = Block::create(rawBlockIter.blockIdfunc, rawBlockIter.blockData);
                }else if(isJEBlock(rawBlockIter.blockIdfunc)){
                    rawBlockIter.block = getJavaBlockMap()[rawBlockIter.blockIdfunc];
                    if (rawBlockIter.blockIdfunc.find("waterlogged=true") != std::string::npos) {
                        rawBlockIter.exBlock = const_cast<Block*>(VanillaBlocks::mStillWater);
                    }
                }
            }
        }
    }

    bool BlockPattern::hasBlock(Block* block) {
        for (auto& rawBlock : rawBlocks) {
            if (block->getTypeName() == getBlockName(rawBlock.blockId) &&
                (rawBlock.blockData < 0 || rawBlock.blockData == block->getTileData())) {
                return true;
            }
        }
        return false;
    }
}  // namespace worldedit