
#pragma once

#include "Pattern.h"

namespace we {
class ClipboardPattern : public Pattern {
public:
    BlockPos         bias;
    class Clipboard* clipboard;

    ClipboardPattern(std::string_view str, std::string_view xuid);

    bool hasBlock(class Block const* block) override;

    bool setBlock(
        const phmap::flat_hash_map<::std::string, double>& variables,
        class EvalFunctions&                               funcs,
        BlockSource*                                       blockSource,
        BlockPos const&                                    pos
    ) override;
};
} // namespace we