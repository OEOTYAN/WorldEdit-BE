#pragma once

#include "LocalContext.h"

#include <ll/api/data/KeyValueDB.h>
#include <ll/api/event/ListenerBase.h>
#include <mc/common/FacingID.h>
#include <mc/platform/UUID.h>

class Player;
class ItemStack;
class CommandOrigin;

namespace we {
class LocalContextManager : public std::enable_shared_from_this<LocalContextManager> {
    WorldEdit& mod;

    ll::data::KeyValueDB storagedState;

    std::vector<ll::event::ListenerPtr> listeners;

    ll::ConcurrentDenseMap<mce::UUID, std::shared_ptr<LocalContext>> playerStates;

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
    LocalContextManager(WorldEdit&);
    ~LocalContextManager();

    // bool has(mce::UUID const& uuid, bool temp = false);

    std::shared_ptr<LocalContext> get(mce::UUID const& uuid, bool temp = false);

    std::shared_ptr<LocalContext> getOrCreate(mce::UUID const& uuid, bool temp = false);

    std::shared_ptr<LocalContext> get(CommandOrigin const&);

    std::shared_ptr<LocalContext> getOrCreate(CommandOrigin const&);

    void remove(mce::UUID const& uuid);

    void removeTemps();
};
} // namespace we
