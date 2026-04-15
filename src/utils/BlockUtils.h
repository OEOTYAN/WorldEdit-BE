#pragma once

#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/VanillaBlockTypeIds.h>
#include <mc/world/level/block/BlockChangeContext.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>

namespace we {
class DirectAccessBlocks {
public:
    static Block const& Air() {
        return *BlockTypeRegistry::get().getDirectAccessBlocks().mAirBlock.mDefaultState;
    }
};

inline BlockChangeContext bcc{};

inline bool hasBlockState(Block const& block, HashedString const& stateName) {
    return block.getBlockType().getBlockState(stateName) != nullptr;
}

} // namespace we
