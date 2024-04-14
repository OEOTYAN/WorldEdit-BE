#pragma once

#include "PlayerState.h"

#include <ll/api/data/KeyValueDB.h>
#include <ll/api/event/ListenerBase.h>
#include <mc/deps/core/mce/UUID.h>
#include <mc/enums/FacingID.h>

class Player;
class ItemStack;
class CommandOrigin;

namespace we {
class PlayerStateManager : public std::enable_shared_from_this<PlayerStateManager> {
    ll::data::KeyValueDB storagedState;

    std::vector<ll::event::ListenerPtr> listeners;

    pfh_map<mce::UUID, std::shared_ptr<PlayerState>> playerStates;

    bool release(mce::UUID const& uuid);

    enum class ClickState {
        Pass,
        Hold,
    };

    ClickState playerLeftClick(
        Player&                  player,
        bool                     isLong,
        ItemStack const&         item,
        WithDim<BlockPos> const& dst,
        FacingID                 mFace
    );
    ClickState playerRightClick(
        Player&                  player,
        bool                     isLong,
        ItemStack const&         item,
        WithDim<BlockPos> const& dst,
        FacingID                 mFace
    );

public:
    PlayerStateManager();
    ~PlayerStateManager();

    // bool has(mce::UUID const& uuid, bool temp = false);

    std::shared_ptr<PlayerState> get(mce::UUID const& uuid, bool temp = false);

    std::shared_ptr<PlayerState> getOrCreate(mce::UUID const& uuid, bool temp = false);

    std::shared_ptr<PlayerState> get(CommandOrigin const&);

    std::shared_ptr<PlayerState> getOrCreate(CommandOrigin const&);

    void remove(mce::UUID const& uuid);

    void removeTemps();
};
} // namespace we
