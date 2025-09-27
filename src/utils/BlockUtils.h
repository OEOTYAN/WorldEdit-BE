#pragma once

#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>

namespace we {
class DirectAccessBlocks {
public:
    static Block const& Air() {
        return *BlockTypeRegistry::get().getDirectAccessBlocks().mAirBlock.mDefaultState;
    }
};
} // namespace we

inline bool operator==(Block const& lhs, Block const& rhs) {
    return lhs.mSerializationIdHash == rhs.mSerializationIdHash;
}
