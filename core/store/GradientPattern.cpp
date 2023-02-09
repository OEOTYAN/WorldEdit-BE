//
// Created by OEOTYAN on 2023/02/02.
//

#include "GradientPattern.h"
#include "WorldEdit.h"
#include "utils/StringTool.h"
#include "utils/StringHelper.h"
#include "Nlohmann/json.hpp"

namespace worldedit {
    GradientPattern::GradientPattern(std::string str, std::string xuid) : Pattern(xuid) {
        type = Pattern::PatternType::GRADIENT;
        if (frontIs(str, "#lighten")) {
            lighten = true;
        }

        nlohmann::json list;
        std::ifstream i(WE_DIR + "mappings/block_gradient.json");
        i >> list;

        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<class Block*>>> tmpMap;
        for (auto& g : list.items()) {
            std::string keyGroup = g.key();
            if (!g.value().is_object()) {
                continue;
            }
            for (auto& k : g.value().items()) {
                std::string key = k.key();
                if (!k.value().is_array()) {
                    continue;
                }
                auto& blockVector = tmpMap[keyGroup][keyGroup + ":" + key];
                blockVector.clear();
                for (auto& b : k.value()) {
                    if (!b.is_string()) {
                        continue;
                    }
                    Block* block = tryGetBlockFromAllVersion(b);
                    if (block == nullptr) {
                        // logger().debug(static_cast<std::string>(b));
                        continue;
                    }
                    blockVector.push_back(block);
                }
            }
        }

        if (lighten) {
            str = str.substr(8);
        } else {
            str = str.substr(7);
        }
        if (str.length() == 0) {
            blockGradientMap = tmpMap["nc"];
        } else {
            if (str.front() == '[') {
                str = str.substr(1);
            }
            if (str.back() == ']') {
                str = str.substr(0, str.length() - 1);
            }
            auto tmpVec = SplitStrWithPattern(str, ",");
            std::unordered_set<std::string> op(tmpVec.begin(), tmpVec.end());
            if (op.find("!nc") == op.end()) {
                op.insert("nc");
            }
            for (auto& g : tmpMap) {
                if (op.find(g.first) != op.end()) {
                    blockGradientMap.insert(g.second.begin(), g.second.end());
                }
            }
            for (auto& p : op) {
                if (p[0] == '!' && blockGradientMap.find(p.substr(1)) != blockGradientMap.end()) {
                    blockGradientMap.erase(p.substr(1));
                }
            }
        }
        for (auto& [vName, vec] : blockGradientMap) {
            for (int i = 0; i < vec.size(); i++) {
                // logger().debug("{}, {}:{}, {}", vName, vec[i]->getTypeName(), vec[i]->getTileData(), i);
                gradientNameMap[vec[i]] = std::make_pair(vName, i);
            }
        }
    }

    class Block* GradientPattern::getBlock(const std::unordered_map<::std::string, double>& variables,
                                           class EvalFunctions& funcs) {
        return nullptr;
    }

    bool GradientPattern::hasBlock(class Block* block) {
        return gradientNameMap.find(block) != gradientNameMap.end();
    }

    bool GradientPattern::setBlock(const std::unordered_map<::std::string, double>& variables,
                                   class EvalFunctions& funcs,
                                   BlockSource* blockSource,
                                   const BlockPos& pos) {
        Block* block = const_cast<Block*>(&blockSource->getBlock(pos));
        if (hasBlock(block)) {
            auto [name, iter] = gradientNameMap[block];
            auto& blocklist = blockGradientMap[name];
            // logger().debug("{}, {}:{}, {}", name, block->getTypeName(), block->getTileData(), iter);
            if (lighten && iter >= 1) {
                --iter;
                // logger().debug("{}:{}",  blocklist[iter]->getTypeName(), blocklist[iter]->getTileData());
                return playerData->setBlockSimple(blockSource, funcs, variables, pos, blocklist[iter]);
            } else if (!lighten) {
                ++iter;
                if (iter < blocklist.size()) {
                    // logger().debug("{}:{}", blocklist[iter]->getTypeName(), blocklist[iter]->getTileData());
                    return playerData->setBlockSimple(blockSource, funcs, variables, pos, blocklist[iter]);
                }
            }
        }
        return false;
    }
}  // namespace worldedit