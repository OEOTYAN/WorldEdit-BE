//
// Created by OEOTYAN on 2022/05/18.
//
#pragma once

#include "Global.h"
#include "eval/Eval.h"

namespace worldedit {

    class Pattern {
       public:
        enum class PatternType {
            NONE,
            BLOCKLIST,
            HAND,
            CLIPBOARD,
            GRADIENT,
        };

        class PlayerData* playerData = nullptr;

        PatternType type = PatternType::NONE;

        Pattern() = default;
        Pattern(std::string xuid);

        virtual class Block* getBlock(const std::unordered_map<::std::string, double>& variables,
                                      class EvalFunctions& funcs) {
            return nullptr;
        }

        virtual bool hasBlock(class Block* block) { return false; }

        virtual bool setBlock(const std::unordered_map<::std::string, double>& variables,
                              class EvalFunctions& funcs,
                              BlockSource* blockSource,
                              const BlockPos& pos) {
            return false;
        }

        static std::unique_ptr<Pattern> createPattern(const std::string& p, std::string xuid);
    };
}  // namespace worldedit