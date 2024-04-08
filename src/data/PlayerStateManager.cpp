#include "PlayerStateManager.h"
#include "worldedit/WorldEdit.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerConnectEvent.h>
#include <ll/api/event/player/PlayerLeaveEvent.h>

#include <ll/api/event/player/PlayerDestroyBlockEvent.h>
#include <ll/api/event/player/PlayerSwingEvent.h>

#include <ll/api/event/player/PlayerInteractBlockEvent.h>
#include <ll/api/event/player/PlayerPlaceBlockEvent.h>
#include <ll/api/event/player/PlayerUseItemEvent.h>
#include <ll/api/event/player/PlayerUseItemOnEvent.h>

#include <ll/api/service/Bedrock.h>
#include <mc/world/item/VanillaItemNames.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/actor/BlockActor.h>
#include <mc/world/level/dimension/Dimension.h>
#include <ll/api/utils/ErrorUtils.h>

namespace we {

static auto saveDelayTime = 1h;

PlayerStateManager::PlayerStateManager()
: storagedState(WorldEdit::getInstance().getSelf().getDataDir() / u8"player_states") {
    using namespace ll::event;
    auto& bus = EventBus::getInstance();
    listeners.emplace_back(
        bus.emplaceListener<PlayerConnectEvent>([this](PlayerConnectEvent& ev) {
            WorldEdit::getInstance().getPool().addTask([id  = ev.self().getUuid(),
                                                        ptr = weak_from_this()] {
                if (!ptr.expired()) ptr.lock()->getOrCreate(id);
            });
        })
    );
    listeners.emplace_back(
        bus.emplaceListener<PlayerLeaveEvent>([this](PlayerLeaveEvent& ev) {
            WorldEdit::getInstance().getScheduler().add<ll::schedule::DelayTask>(
                saveDelayTime,
                [id = ev.self().getUuid(), ptr = weak_from_this()] {
                    if (!ptr.expired()) ptr.lock()->release(id);
                }
            );
        })
    );
    listeners.emplace_back(
        bus.emplaceListener<PlayerDestroyBlockEvent>([this](PlayerDestroyBlockEvent& ev) {
            if (!ev.self().isOperator() || !ev.self().isCreative()) {
                return;
            }
            ev.setCancelled(!playerLeftClick(
                ev.self(),
                false,
                ev.self().getSelectedItem(),
                {ev.pos(), ev.self().getDimensionId()},
                FacingID::Unknown
            ));
            if (ev.isCancelled()) {
                WorldEdit::getInstance().geServerScheduler().add<ll::schedule::DelayTask>(
                    1_tick,
                    [dst = WithDim<BlockPos>{ev.pos(), ev.self().getDimensionId()}] {
                        auto dim = ll::service::getLevel()->getDimension(dst.dim);
                        if (dim) {
                            return;
                        }
                        auto blockActor =
                            dim->getBlockSourceFromMainChunkSource().getBlockEntity(
                                dst.pos
                            );
                        if (blockActor) {
                            blockActor->refresh(dim->getBlockSourceFromMainChunkSource());
                        }
                    }
                );
            }
        })
    );
    listeners.emplace_back(bus.emplaceListener<PlayerSwingEvent>([this](
                                                                     PlayerSwingEvent& ev
                                                                 ) {
        if (!ev.self().isOperator() || !ev.self().isCreative()) {
            return;
        }
        auto hit = ev.self().traceRay(
            (float)WorldEdit::getInstance().getConfig().player_state.maximum_brush_length,
            false
        );
        if (!hit) {
            return;
        }
        if (hit.mIsHitLiquid && !ev.self().isImmersedInWater()) {
            hit.mBlockPos = hit.mLiquidPos;
            hit.mFacing   = hit.mLiquidFacing;
        }
        playerLeftClick(
            ev.self(),
            true,
            ev.self().getSelectedItem(),
            {hit.mBlockPos, ev.self().getDimensionId()},
            hit.mFacing
        );
    }));
    listeners.emplace_back(bus.emplaceListener<PlayerInteractBlockEvent>(
        [this](PlayerInteractBlockEvent& ev) {
            if (!ev.self().isOperator() || !ev.self().isCreative()) {
                return;
            }
            ev.setCancelled(!playerRightClick(
                ev.self(),
                false,
                ev.self().getSelectedItem(),
                {ev.pos(), ev.self().getDimensionId()},
                FacingID::Unknown
            ));
        }
    ));
    listeners.emplace_back(
        bus.emplaceListener<PlayerPlacingBlockEvent>([this](PlayerPlacingBlockEvent& ev) {
            if (!ev.self().isOperator() || !ev.self().isCreative()) {
                return;
            }
            ev.setCancelled(!playerRightClick(
                ev.self(),
                false,
                ev.self().getSelectedItem(),
                {ev.pos(), ev.self().getDimensionId()},
                FacingID::Unknown
            ));
        })
    );
    listeners.emplace_back(
        bus.emplaceListener<PlayerUseItemOnEvent>([this](PlayerUseItemOnEvent& ev) {
            if (!ev.self().isOperator() || !ev.self().isCreative()) {
                return;
            }
            ev.setCancelled(!playerRightClick(
                ev.self(),
                false,
                ev.item(),
                {ev.blockPos(), ev.self().getDimensionId()},
                (FacingID)ev.face()
            ));
        })
    );
    listeners.emplace_back(bus.emplaceListener<
                           PlayerUseItemEvent>([this](PlayerUseItemEvent& ev) {
        if (!ev.self().isOperator() || !ev.self().isCreative()) {
            return;
        }
        auto hit = ev.self().traceRay(
            (float)WorldEdit::getInstance().getConfig().player_state.maximum_brush_length,
            false
        );
        if (!hit) {
            return;
        }
        if (hit.mIsHitLiquid && !ev.self().isImmersedInWater()) {
            hit.mBlockPos = hit.mLiquidPos;
            hit.mFacing   = hit.mLiquidFacing;
        }
        playerRightClick(
            ev.self(),
            true,
            ev.item(),
            {hit.mBlockPos, ev.self().getDimensionId()},
            hit.mFacing
        );
    }));
}
PlayerStateManager::~PlayerStateManager() {
    using namespace ll::event;
    for (auto& l : listeners) {
        EventBus::getInstance().removeListener(l);
    }
    playerStates.for_each([&](auto&& p) {
        if (!p.second->dirty()) {
            return;
        }
        CompoundTag nbt;
        p.second->serialize(nbt);
        std::unique_lock lock{dbmutex};
        storagedState.set(p.first.asString(), nbt.toBinaryNbt());
    });
}
std::shared_ptr<PlayerState> PlayerStateManager::getOrCreate(mce::UUID const& uuid) {
    std::shared_ptr<PlayerState> res;
    playerStates.lazy_emplace_l(
        uuid,
        [&](auto&& p) { res = p.second; },
        [&, this](auto&& ctor) {
            std::optional<std::string> nbt;
            {
                std::shared_lock lock{dbmutex};
                nbt = storagedState.get(uuid.asString());
            }
            res = std::make_shared<PlayerState>(uuid);
            if (nbt) {
                try{
                    res->deserialize(*CompoundTag::fromBinaryNbt(*nbt));
                }catch(...){
                    ll::error_utils::printCurrentException(WorldEdit::getInstance().getLogger());
                }
            }
            ctor(uuid, res);
        }
    );
    res->lastUsedTime = std::chrono::system_clock::now();
    return res;
}
bool PlayerStateManager::release(mce::UUID const& uuid) {
    return playerStates.erase_if(uuid, [&](auto&& p) {
        if (std::chrono::system_clock::now() - p.second->lastUsedTime.load()
            <= saveDelayTime) {
            return false;
        }
        if (!p.second->dirty()) {
            return true;
        }
        CompoundTag nbt;
        p.second->serialize(nbt);
        std::unique_lock lock{dbmutex};
        return storagedState.set(uuid.asString(), nbt.toBinaryNbt());
    });
}


bool PlayerStateManager::playerLeftClick(
    Player&                  player,
    bool                     isLong,
    ItemStack const&         item,
    WithDim<BlockPos> const& dst,
    FacingID                 mFace
) {
    auto  data    = getOrCreate(player.getUuid());
    auto& current = player.getLevel().getCurrentTick();
    bool  needDiscard{};
    if (current.t - data->lastLeftClick.load().t
        < WorldEdit::getInstance().getConfig().player_state.minimum_response_tick) {
        needDiscard = true;
    }
    auto& itemName = item.getFullNameHash();
    if (itemName == VanillaItemNames::WoodenAxe) {
        if (isLong) {
            return true;
        }
        data->lastLeftClick = current;
        if (!needDiscard) {
            data->setMainPos(dst);
        }
        return false;
    }
    return true;
}
bool PlayerStateManager::playerRightClick(
    Player&                  player,
    bool                     isLong,
    ItemStack const&         item,
    WithDim<BlockPos> const& dst,
    FacingID                 mFace
) {
    auto  data    = getOrCreate(player.getUuid());
    auto& current = player.getLevel().getCurrentTick();
    bool  needDiscard{};
    if (current.t - data->lastRightClick.load().t
        < WorldEdit::getInstance().getConfig().player_state.minimum_response_tick) {
        needDiscard = true;
    }
    auto& itemName = item.getFullNameHash();
    if (itemName == VanillaItemNames::WoodenAxe) {
        if (isLong) {
            return true;
        }
        data->lastRightClick = current;
        if (!needDiscard) {
            data->setVicePos(dst);
        }
        return false;
    }
    return true;
}

} // namespace we
