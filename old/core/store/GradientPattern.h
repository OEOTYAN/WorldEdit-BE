//
// Created by OEOTYAN on 2023/02/02.
//

#pragma once

#include "Pattern.h"

namespace worldedit {
    class GradientPattern final : public Pattern {
       public:
        phmap::flat_hash_map<class Block const*, std::pair<std::string, int>> gradientNameMap;
        phmap::flat_hash_map<std::string, std::vector<class Block const*>> blockGradientMap;

        bool lighten = false;

        GradientPattern(std::string_view str, std::string_view xuid);

        class Block const* getBlock(const phmap::flat_hash_map<::std::string, double>& variables,
                              class EvalFunctions& funcs) override;

        bool hasBlock(class Block const* block) override;

        bool setBlock(const phmap::flat_hash_map<::std::string, double>& variables,
                      class EvalFunctions& funcs,
                      BlockSource* blockSource,
                      const BlockPos& pos) override;
    };
}  // namespace worldedit