#include <mc/world/level/block/registry/BlockTypeRegistry.h>

namespace we {
class DirectAccessBlocks {
public:
    static Block const& Air() {
        return *BlockTypeRegistry::get().getDirectAccessBlocks().mAirBlock.mDefaultState;
    }
};
} // namespace we
