#pragma once

#include "Globals.h"
#include <mc/Level.hpp>
// #include <mc/BlockInstance.hpp>
#include "particle/Graphics.h"
#include <mc/BedrockBlocks.hpp>
#include <mc/Block.hpp>
#include <mc/BlockActor.hpp>
#include <mc/BlockSource.hpp>
#include <mc/CommandUtils.hpp>
#include <mc/CompoundTag.hpp>
#include <mc/StructureSettings.hpp>
#include <mc/VanillaBlockStateTransformUtils.hpp>

namespace we {

class BlockNBTSet {
public:
    std::unique_ptr<CompoundTag> blockEntity = nullptr;
    std::optional<std::pair<class Block const*, class Block const*>> blocks;
    std::optional<int>                                               biomeId;
    BlockNBTSet() = default;
    BlockNBTSet(BlockNBTSet const&);
    BlockNBTSet(BlockNBTSet&&) = default;
    BlockNBTSet& operator=(BlockNBTSet const&);
    BlockNBTSet& operator=(BlockNBTSet&&) = default;
    BlockNBTSet(BlockInstance& blockInstance);
    Block const* getBlock() const { return blocks.value().first; }
    Block const* getExBlock() const { return blocks.value().second; }
    bool         setBlockWithoutcheckGMask(
                BlockPos const&   pos,
                BlockSource*      blockSource,
                class PlayerData& data,
                bool              needBiome = false
            ) const;
    bool setBlock(
        BlockPos const&                                  pos,
        BlockSource*                                     blockSource,
        class PlayerData&                                data,
        class EvalFunctions&                             funcs,
        phmap::flat_hash_map<std::string, double> const& var,
        bool                                             needBiome = false
    ) const;
    bool setBlock(
        BlockPos const&                                  pos,
        BlockSource*                                     blockSource,
        class PlayerData&                                data,
        class EvalFunctions&                             funcs,
        phmap::flat_hash_map<std::string, double> const& var,
        Rotation                                         rotation,
        Mirror                                           mirror,
        bool                                             needBiome = false
    ) const;
};
} // namespace we