#pragma once

#include "PlayerContext.h"

#include <ll/api/data/KeyValueDB.h>
#include <ll/api/event/ListenerBase.h>
#include <mc/deps/core/mce/UUID.h>
#include <mc/enums/FacingID.h>

class Player;
class ItemStack;
class CommandOrigin;

namespace we {
class PlayerContextManager : public std::enable_shared_from_this<PlayerContextManager> {
    WorldEdit& mod;

    ll::data::KeyValueDB storagedState;

    std::vector<ll::event::ListenerPtr> listeners;

    ll::ConcurrentDenseMap<mce::UUID, std::shared_ptr<PlayerContext>> playerStates;

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
    PlayerContextManager();
    ~PlayerContextManager();

    // bool has(mce::UUID const& uuid, bool temp = false);

    std::shared_ptr<PlayerContext> get(mce::UUID const& uuid, bool temp = false);

    std::shared_ptr<PlayerContext> getOrCreate(mce::UUID const& uuid, bool temp = false);

    std::shared_ptr<PlayerContext> get(CommandOrigin const&);

    std::shared_ptr<PlayerContext> getOrCreate(CommandOrigin const&);

    void remove(mce::UUID const& uuid);

    void removeTemps();
};
} // namespace we
