#pragma once

#include "Config.h"
#include "region/Region.h"

#include <mc/deps/core/mce/UUID.h>
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
    std::optional<WithGeo<WithDim<BlockPos>>> mainPos;
    std::optional<WithGeo<WithDim<BlockPos>>> offPos;
    std::optional<RegionType>                 regionType;
    std::shared_ptr<Region>                   region;
    Config::PlayerConfig                      config;

    LocalContext(mce::UUID const& uuid, bool temp);

    bool setMainPos(WithDim<BlockPos> const&);
    bool setOffPos(WithDim<BlockPos> const&);

    ll::Expected<> serialize(CompoundTag&) const;
    ll::Expected<> deserialize(CompoundTag const&);
};
} // namespace we
