#pragma once

#include "Config.h"
#include "builder/Builder.h"
#include "data/HistoryManager.h"
#include "expression/expression.h"
#include "region/Region.h"

#include <mc/platform/UUID.h>
#include <mc/server/SimulatedPlayer.h>
#include <mc/server/commands/PlayerPermissionLevel.h>
#include <mc/world/level/Tick.h>

namespace we {
class LocalContextManager;
class LocalContext {
    friend LocalContextManager;
    std::atomic<Tick> mutable lastLeftClick;
    std::atomic<Tick> mutable lastRightClick;

    bool const      temp;
    mce::UUID const uuid;

    void setMainPosInternal();
    void setOffPosInternal();

    Region& getOrCreateRegion(WithDim<BlockPos> const&);

public:
    mutable Vec3          lastPos;
    mutable DimensionType lastDim;

    std::optional<WithGeo<WithDim<BlockPos>>> mainPos;
    std::optional<WithGeo<WithDim<BlockPos>>> offPos;
    std::optional<RegionType>                 regionType;
    std::shared_ptr<Region>                   region;
    Config::PlayerConfig                      config;
    HistoryManager                            history;
    std::shared_ptr<Builder>                  builder;

    mce::UUID getUuid() const { return uuid; }

    LocalContext(mce::UUID const& uuid, bool temp);
    ~LocalContext();

    void setupBuilder(BuilderType type);

    void updateBuilderByPerm(bool isOp);

    bool setMainPos(WithDim<BlockPos> const&);
    bool setOffPos(WithDim<BlockPos> const&);

    SimulatedPlayer* createSimulatedPlayer(std::string const& name) const;

    bool masked(BlockSource&, BlockPos const&) const;

    bool setBlock(
        BlockSource&,
        BlockPos const&,
        Block const&,
        std::shared_ptr<BlockActor>
    ) const;
    bool setExtraBlock(BlockSource&, BlockPos const&, Block const&) const;
    bool setBiome(BlockSource&, BlockPos const&, Biome const&) const;

    ll::Expected<> serialize(CompoundTag&) const noexcept;
    ll::Expected<> deserialize(CompoundTag const&) noexcept;
};
} // namespace we
