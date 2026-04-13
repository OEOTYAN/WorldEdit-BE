#pragma once

#include "pattern/PatternAst.h"
#include "worldedit/Global.h"

namespace we {
class PatternParser {
public:
    static ll::Expected<PatternAst> parse(std::string_view input);
};
} // namespace we
