
#include "ClipboardPattern.h"
#include "Clipboard.hpp"
#include "WorldEdit.h"
#include "region/Region.h"

namespace we {

ClipboardPattern::ClipboardPattern(std::string_view str, std::string_view xuid)
: Pattern(xuid) {
    type = Pattern::PatternType::CLIPBOARD;
    if (!playerData->clipboard.used) {
        return;
    }
    clipboard    = &playerData->clipboard;
    auto& region = playerData->region;
    if (region != nullptr) {
        if (str.find("@c") != std::string::npos) {
            bias = region->getCenter().toBlockPos();
        } else {
            bias = region->getBoundBox().min;
        }
    } else {
        bias = BlockPos(0, 0, 0);
    }
    auto             strSize = str.size();
    size_t           mi      = 0;
    std::vector<int> poslist;
    poslist.clear();
    while (mi < strSize) {
        if (str[mi] == '-' || isdigit(str[mi])) {
            auto head = mi;
            mi++;
            while (mi < strSize && (isdigit(str[mi]))) {
                mi++;
            }
            poslist.push_back(std::stoi(asString(str.substr(head, mi - head))));
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
bool ClipboardPattern::hasBlock(class Block const* block) {
    for (auto& x : clipboard->blockslist) {
        if (x.blocks.has_value() && block == x.blocks.value().first) {
            return true;
        }
    }
    return false;
}

bool ClipboardPattern::setBlock(
    const phmap::flat_hash_map<::std::string, double>& variables,
    class EvalFunctions&                               funcs,
    BlockSource*                                       blockSource,
    BlockPos const&                                    pos
) {
    if (clipboard != nullptr) {
        return clipboard->getSetLoop(pos - bias)
            .setBlock(pos, blockSource, *playerData, funcs, variables);
    }
    return false;
}
} // namespace we