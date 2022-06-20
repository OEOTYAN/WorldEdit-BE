//
// Created by OEOTYAN on 2022/06/10.
//
#include "BlockPattern.hpp"

namespace worldedit {

    std::string RawBlock::getFxxkBlockName(int id) {
        return getBlockName(id);
    };

    BlockPattern::BlockPattern(std::string str, std::string xuid, Region* region) {
        if (str.find("#clipboard") != std::string::npos) {
            auto& mod = worldedit::getMod();
            if (mod.playerClipboardMap.find(xuid) == mod.playerClipboardMap.end()) {
                return;
            }
            clipboard = &mod.playerClipboardMap[xuid];
            if (str.find("@c") != std::string::npos) {
                auto center = region->getCenter();
                bias.x      = static_cast<int>(floor(center.x));
                bias.y      = static_cast<int>(floor(center.y));
                bias.z      = static_cast<int>(floor(center.z));
            } else {
                bias = region->getBoundBox().min;
            }
            size_t           mi = 0;
            std::vector<int> poslist;
            poslist.clear();
            while (mi < str.size()) {
                if (str[mi] == '-' || isdigit(str[mi])) {
                    auto head7 = mi;
                    mi++;
                    while (mi < str.size() && (isdigit(str[mi]))) {
                        mi++;
                    }
                    poslist.push_back(std::stoi(str.substr(head7, mi - head7)));
                }
                mi++;
            }
            if (poslist.size() > 0) {
                bias.x -= poslist[0];
            }
            if (poslist.size() > 1) {
                bias.y -= poslist[1];
            }
            if (poslist.size() > 2) {
                bias.z -= poslist[2];
            }
            return;
        }
        std::vector<std::string> raw;
        raw.clear();
        string form = "";
        size_t i    = 0;
        while (i < str.size()) {
            if (str[i] == '-' || isdigit(str[i])) {
                auto head = i;
                i++;
                while (i < str.size() && (str[i] == '.' || isdigit(str[i]))) {
                    i++;
                }
                form += string("num") + str[i];
                raw.push_back(str.substr(head, i - head));
            } else if (str[i] == '\'') {
                i++;
                auto head = i;
                while (i < str.size() && (str[i] != '\'')) {
                    i++;
                }
                raw.push_back(str.substr(head, i - head));
                i++;
                form += string("funciton") + str[i];
            } else if (isalpha(str[i])) {
                auto head = i;
                i++;
                while (i < str.size() && (isalpha(str[i]) || str[i] == '_' || isdigit(str[i]) ||
                                          (str[i] == ':' && i + 1 < str.size() && isalpha(str[i + 1])))) {
                    i++;
                }
                form += string("block") + str[i];
                raw.push_back(str.substr(head, i - head));
            } else if (str[i] == '{') {
                auto head    = i;
                int  bracket = -1;
                i++;
                while (i < str.size() && bracket < 0) {
                    if (str[i] == '{') {
                        bracket--;
                    } else if (str[i] == '}') {
                        bracket++;
                    }
                    i++;
                }
                form += string("SNBT") + str[i];
                raw.push_back(str.substr(head, i - head));
            }
            i++;
        }
        auto blockList = SplitStrWithPattern(form, ",");
        blockNum       = blockList.size();
        percents.resize(blockNum);
        rawBlocks.resize(blockNum);
        int rawPtr = 0;
        for (int iter = 0; iter < blockNum; iter++) {
            if (blockList[iter].find("%") != std::string::npos) {
                if (blockList[iter].find("num%") != std::string::npos) {
                    percents[iter].value = std::stod(raw[rawPtr]);
                } else {
                    percents[iter].isNum    = false;
                    percents[iter].function = raw[rawPtr];
                }
                rawPtr++;
            } else {
                blockList[iter] = "%" + blockList[iter];
            }
            if (blockList[iter].find("%num") != std::string::npos) {
                rawBlocks[iter].blockId = std::stoi(raw[rawPtr]);
                rawPtr++;
            } else if (blockList[iter].find("%block") != std::string::npos) {
                std::string tmpName = raw[rawPtr];
                if (tmpName.find("minecraft:") == std::string::npos) {
                    tmpName = "minecraft:" + tmpName;
                }
                rawBlocks[iter].blockId = getBlockId(tmpName);
                rawPtr++;
            } else if (blockList[iter].find("%funciton") != std::string::npos) {
                rawBlocks[iter].blockId     = -1;
                rawBlocks[iter].constBlock  = false;
                rawBlocks[iter].blockIdfunc = raw[rawPtr];
                rawPtr++;
            } else if (blockList[iter].find("%SNBT") != std::string::npos) {
                rawBlocks[iter].blockId   = -1;
                rawBlocks[iter].blockData = -1;
                std::string tmpSNBT       = raw[rawPtr];
                if (tmpSNBT[0] == '{' && tmpSNBT[1] == '{') {
                    size_t                   i2       = 1;
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
                            std::cout << tmpSNBT.substr(head2, i2 - head2) << std::endl;
                        }
                        i2++;
                    }
                    rawBlocks[iter].block = Block::create(CompoundTag::fromSNBT(tmpSNBTs[0]).get());
                    if (tmpSNBTs.size() > 1) {
                        rawBlocks[iter].exBlock = Block::create(CompoundTag::fromSNBT(tmpSNBTs[1]).get());
                    }
                    if (tmpSNBTs.size() > 2) {
                        rawBlocks[iter].blockEntity = CompoundTag::fromSNBT(tmpSNBTs[2])->toBinaryNBT();
                    }
                } else {
                    rawBlocks[iter].block = Block::create(CompoundTag::fromSNBT(tmpSNBT).get());
                }
                rawPtr++;
                continue;
            }
            if (blockList[iter].find(":num") != std::string::npos) {
                rawBlocks[iter].blockData = std::stoi(raw[rawPtr]);
                rawPtr++;
            } else if (blockList[iter].find(":funciton") != std::string::npos) {
                rawBlocks[iter].blockData     = -1;
                rawBlocks[iter].constBlock    = false;
                rawBlocks[iter].blockDatafunc = raw[rawPtr];
                rawPtr++;
            }
            if (rawBlocks[iter].constBlock) {
                rawBlocks[iter].block = Block::create(getBlockName(rawBlocks[iter].blockId), rawBlocks[iter].blockData);
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