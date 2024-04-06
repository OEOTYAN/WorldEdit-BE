#pragma once

#include "Globals.h"
#include "eval/Eval.h"

namespace we {

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
    Pattern(std::string_view xuid);

    virtual class Block const* getBlock(
        const phmap::flat_hash_map<::std::string, double>& variables,
        class EvalFunctions&                               funcs
    ) {
        return nullptr;
    }

    virtual bool hasBlock(class Block const* block) { return false; }

    virtual bool setBlock(
        const phmap::flat_hash_map<::std::string, double>& variables,
        class EvalFunctions&                               funcs,
        BlockSource*                                       blockSource,
        BlockPos const&                                    pos
    ) {
        return false;
    }

    static std::unique_ptr<Pattern>
    createPattern(std::string_view p, std::string_view xuid);
};
} // namespace we