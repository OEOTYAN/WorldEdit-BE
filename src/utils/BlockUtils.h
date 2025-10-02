#pragma once

#include <mc/world/level/block/BedrockBlockNames.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>

namespace we {
class DirectAccessBlocks {
public:
    static Block const& Air() {
        return *BlockTypeRegistry::get().getDirectAccessBlocks().mAirBlock.mDefaultState;
    }
};
inline bool isAir(Block const& block) {
    return block.getBlockType().mNameInfo->mFullName->mStrHash
        == BedrockBlockNames::Air().mStrHash;
}

} // namespace we
