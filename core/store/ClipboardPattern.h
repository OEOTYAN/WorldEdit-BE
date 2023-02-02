//
// Created by OEOTYAN on 2023/02/02.
//

#pragma once

#include "Pattern.h"

namespace worldedit {
    class ClipboardPattern : public Pattern {
       public:
        BlockPos bias;
        class Clipboard* clipboard;

        ClipboardPattern(std::string str, std::string xuid);

        bool hasBlock(class Block* block) override;

        bool setBlock(const std::unordered_map<::std::string, double>& variables,
                      class EvalFunctions& funcs,
                      BlockSource* blockSource,
                      const BlockPos& pos) override;
    };
}  // namespace worldedit