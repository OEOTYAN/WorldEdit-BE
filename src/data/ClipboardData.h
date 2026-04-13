#pragma once

#include "worldedit/Global.h"

namespace we {
struct ClipboardData {
    BlockPos                  size{0, 0, 0};
    std::vector<HashedString> blocks;

    bool empty() const {
        return blocks.empty() || size.x < 0 || size.y < 0 || size.z < 0;
    }

    BlockPos wrap(BlockPos const& pos) const {
        auto wrapAxis = [](int value, int maxInclusive) -> int {
            auto length = maxInclusive + 1;
            if (length <= 0) {
                return 0;
            }
            auto mod = value % length;
            return mod < 0 ? mod + length : mod;
        };

        return BlockPos{
            wrapAxis(pos.x, size.x),
            wrapAxis(pos.y, size.y),
            wrapAxis(pos.z, size.z),
        };
    }

    size_t index(BlockPos const& pos) const {
        return static_cast<size_t>(pos.x)
             + static_cast<size_t>(size.x + 1)
                   * (static_cast<size_t>(pos.y)
                      + static_cast<size_t>(size.y + 1) * static_cast<size_t>(pos.z));
    }

    optional_ref<Block const> getBlock(BlockPos const& pos) const {
        auto wrapped = wrap(pos);
        auto idx     = index(wrapped);
        if (idx >= blocks.size()) {
            return nullptr;
        }
        return Block::tryGetFromRegistry(blocks[idx]);
    }
};
} // namespace we