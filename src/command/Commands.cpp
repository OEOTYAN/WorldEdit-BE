#include "command/Commands.h"
#include "worldedit/WorldEdit.h"

namespace we {

std::vector<std::pair<std::string, std::function<void()>>> setupCommandFunctions;

void setupCommands() {
    for (auto& [name, fn] : setupCommandFunctions) {
        WEDEBUG("setup command {}", name);
        try {
            fn();
        } catch (...) {
            logger().error("threw from setup command {}", name);
            ll::error_utils::printCurrentException(logger());
        }
        WEDEBUG("setup command {} done", name);
    }
}

bool addSetup(std::string name, std::function<void()> fn) noexcept {
    setupCommandFunctions.emplace_back(std::move(name), std::move(fn));
    return true;
}

} // namespace we