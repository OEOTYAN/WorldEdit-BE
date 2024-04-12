#pragma once

#include "PlayerState.h"

#include <ll/api/data/KeyValueDB.h>
#include <ll/api/event/ListenerBase.h>
#include <mc/deps/core/mce/UUID.h>
#include <mc/enums/FacingID.h>

class Player;
class ItemStack;

namespace we {
class PlayerStateManager : public std::enable_shared_from_this<PlayerStateManager> {
    ll::data::KeyValueDB storagedState;
    std::shared_mutex    dbmutex;

    std::vector<ll::event::ListenerPtr> listeners;

    pfh_map<mce::UUID, std::shared_ptr<PlayerState>> playerStates;

    bool release(mce::UUID const& uuid);

    bool playerLeftClick(
        Player&                  player,
        bool                     isLong,
        ItemStack const&         item,
        WithDim<BlockPos> const& dst,
        FacingID                 mFace
    );
    bool playerRightClick(
        Player&                  player,
        bool                     isLong,
        ItemStack const&         item,
        WithDim<BlockPos> const& dst,
        FacingID                 mFace
    );

public:
    PlayerStateManager();
    ~PlayerStateManager();

    std::shared_ptr<PlayerState> getOrCreate(mce::UUID const& uuid);

    void remove(mce::UUID const& uuid);
};
} // namespace we
