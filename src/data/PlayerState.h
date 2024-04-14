#pragma once

#include "Config.h"
#include "region/Region.h"

#include <mc/deps/core/mce/UUID.h>
#include <mc/world/level/Tick.h>

namespace we {
class PlayerStateManager;
class PlayerState {
    friend PlayerStateManager;
    std::atomic_bool mutable mDirty;
    std::atomic<std::chrono::system_clock::time_point> mutable lastUsedTime;

    std::atomic<Tick> mutable lastLeftClick;
    std::atomic<Tick> mutable lastRightClick;

    bool temp;

    mce::UUID uuid;

    void setMainPosInternal();
    void setVicePosInternal();

    Region& getOrCreateRegion(WithDim<BlockPos> const&);

public:
    std::optional<WithGeo<WithDim<BlockPos>>> mainPos;
    std::optional<WithGeo<WithDim<BlockPos>>> vicePos;
    std::optional<RegionType> regionType;
    std::shared_ptr<Region>   region;
    Config::PlayerConfig      config;

    bool dirty() const { return mDirty; }

    PlayerState(mce::UUID const& uuid, bool temp);

    bool setMainPos(WithDim<BlockPos> const&);
    bool setVicePos(WithDim<BlockPos> const&);

    void serialize(CompoundTag&) const;
    void deserialize(CompoundTag const&);
};
} // namespace we
