#pragma once

#include "worldedit/Global.h"

namespace we {
struct GradientData {
    std::string              namespaceName;
    std::string              gradientName;
    std::vector<std::string> blocks;

    bool empty() const { return blocks.empty(); }
};
} // namespace we