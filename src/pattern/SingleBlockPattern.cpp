#include "pattern/SingleBlockPattern.h"

#include "command/CommandMacro.h"

namespace we {
ll::Expected<std::shared_ptr<SingleBlockPattern>> SingleBlockPattern::create() {
    return std::make_shared<SingleBlockPattern>();
}

BlockOperation SingleBlockPattern::pickBlock(BlockPos const&) const {
    BlockOperation operation;
    operation.block = block;
    return operation;
}
} // namespace we
