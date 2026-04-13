#include "pattern/HandPattern.h"

#include "command/CommandMacro.h"

namespace we {
ll::Expected<std::shared_ptr<HandPattern>> HandPattern::create() {
    return std::make_shared<HandPattern>();
}

ll::Expected<> HandPattern::prepare(CommandContextRef const& ctx) {
    auto player = checkPlayer(ctx);
    if (!player) {
        return ll::makeStringError("#hand requires a player origin");
    }

    auto const& selectedItem = player->getSelectedItem();
    if (selectedItem.mBlock == nullptr) {
        return ll::makeStringError("#hand requires holding a block item");
    }

    handBlock = selectedItem.mBlock;
    return {};
}

optional_ref<Block const> HandPattern::pickBlock(BlockPos const&) const {
    return handBlock;
}
} // namespace we
