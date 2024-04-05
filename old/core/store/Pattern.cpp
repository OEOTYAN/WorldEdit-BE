//
// Created by OEOTYAN on 2022/06/10.
//

#include "Patterns.h"
#include "WorldEdit.h"
#include "utils/StringTool.h"

namespace worldedit {

    Pattern::Pattern(std::string_view xuid) {
        playerData = &getPlayersData(xuid);
    }

    std::unique_ptr<Pattern> Pattern::createPattern(std::string_view p, std::string_view xuid) {
        if (frontIs(p, "#hand")) {
            return std::make_unique<BlockListPattern>(BlockListPattern(p, xuid));
        } else if (frontIs(p, "#lighten") || frontIs(p, "#darken")) {
            return std::make_unique<GradientPattern>(GradientPattern(p, xuid));
        } else if (frontIs(p, "#clipboard")) {
            return std::make_unique<ClipboardPattern>(ClipboardPattern(p, xuid));
        } else {
            return std::make_unique<BlockListPattern>(BlockListPattern(p, xuid));
        }
        return nullptr;
    }

}  // namespace worldedit