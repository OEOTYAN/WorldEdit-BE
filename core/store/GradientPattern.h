//
// Created by OEOTYAN on 2023/02/02.
//

#pragma once

#include "Pattern.h"

namespace worldedit {
    class GradientPattern : public Pattern {
       public:
        std::unordered_map<class Block*, std::pair<std::string, int>> gradientNameMap;
        std::unordered_map<std::string, std::vector<class Block*>> blockGradientMap;

        bool lighten = false;

        GradientPattern(std::string str, std::string xuid);

        class Block* getBlock(const std::unordered_map<::std::string, double>& variables,
                              class EvalFunctions& funcs) override;

        bool hasBlock(class Block* block) override;

        bool setBlock(const std::unordered_map<::std::string, double>& variables,
                      class EvalFunctions& funcs,
                      BlockSource* blockSource,
                      const BlockPos& pos) override;
    };
}  // namespace worldedit