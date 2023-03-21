//
// Created by OEOTYAN on 2023/02/02.
//

#pragma once

#include "Pattern.h"

namespace worldedit {
    class Percents {
       public:
        bool isNum = true;
        double value = 1;
        std::string function = "1";
        Percents() = default;

        double getPercents(const phmap::flat_hash_map<::std::string, double>& variables, class EvalFunctions& funcs);
    };
    class RawBlock {
       public:
        bool constBlock = true;
        class Block* block;
        class Block* exBlock;
        bool hasBE = false;
        std::string blockEntity = "";
        int blockId = -2140000000;
        int blockData = -2140000000;
        std::string blockIdfunc = "0";
        std::string blockDatafunc = "0";
        RawBlock();

        class Block* getBlock(const phmap::flat_hash_map<::std::string, double>& variables, class EvalFunctions& funcs);
    };

    class BlockListPattern final : public Pattern {
       public:
        size_t blockNum = 0;
        std::vector<Percents> percents;
        std::vector<RawBlock> rawBlocks;

        BlockListPattern(std::string_view str, std::string_view xuid);

        class Block* getBlock(const phmap::flat_hash_map<::std::string, double>& variables,
                              class EvalFunctions& funcs) override;

        bool hasBlock(class Block* block) override;

        bool setBlock(const phmap::flat_hash_map<::std::string, double>& variables,
                      class EvalFunctions& funcs,
                      BlockSource* blockSource,
                      const BlockPos& pos) override;

       private:
        RawBlock* getRawBlock(const phmap::flat_hash_map<::std::string, double>& variables, class EvalFunctions& funcs);
    };

}  // namespace worldedit