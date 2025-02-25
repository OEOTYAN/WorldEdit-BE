#include "LocalContextManager.h"
#include "worldedit/WorldEdit.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerConnectEvent.h>
#include <ll/api/event/player/PlayerDisconnectEvent.h>

#include <ll/api/event/player/PlayerDestroyBlockEvent.h>
#include <ll/api/event/player/PlayerSwingEvent.h>

#include <ll/api/event/player/PlayerInteractBlockEvent.h>
#include <ll/api/event/player/PlayerPlaceBlockEvent.h>
#include <ll/api/event/player/PlayerUseItemEvent.h>

#include <ll/api/service/Bedrock.h>
#include <ll/api/service/GamingStatus.h>
#include <ll/api/service/ServerInfo.h>
#include <ll/api/utils/ErrorUtils.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/world/level/block/actor/BlockActor.h>

namespace we {
LocalContextManager::LocalContextManager(WorldEdit& we)
: mod(we),
  storagedState(mod.getSelf().getDataDir() / u8"player_states") {
    using namespace ll::event;
    if (ll::getGamingStatus() == ll::GamingStatus::Running && ll::service::getLevel()) {
        ll::service::getLevel()->forEachPlayer([this](Player& player) {
            if (!player.isSimulated()) {
                getOrCreate(player.getUuid());
            }
            return true;
        });
    }
    auto& bus = EventBus::getInstance();
    listeners.emplace_back(
        bus.emplaceListener<PlayerConnectEvent>([this](PlayerConnectEvent& ev) {
            if (!ev.self().isSimulated()) {
                ll::thread::ThreadPoolExecutor::getDefault().execute(
                    [id = ev.self().getUuid(), ptr = weak_from_this()] {
                        if (!ptr.expired()) ptr.lock()->getOrCreate(id);
                    }
                );
            }
        })
    );
    listeners.emplace_back(
        bus.emplaceListener<PlayerDisconnectEvent>([this](PlayerDisconnectEvent& ev) {
            ll::thread::ThreadPoolExecutor::getDefault().execute(
                [id = ev.self().getUuid(), ptr = weak_from_this()] {
                    if (!ptr.expired()) ptr.lock()->release(id);
                }
            );
        })
    );
    listeners.emplace_back(
        bus.emplaceListener<PlayerDestroyBlockEvent>([this](PlayerDestroyBlockEvent& ev) {
            ev.setCancelled(
                ClickState::Hold
                == playerLeftClick(
                    ev.self(),
                    false,
                    ev.self().getSelectedItem(),
                    {ev.pos(), ev.self().getDimensionId()},
                    FacingID::Unknown
                )
            );
            if (ev.isCancelled()) {
                ll::thread::ThreadPoolExecutor::getDefault().executeAfter(
                    [dst = WithDim<BlockPos>{ev.pos(), ev.self().getDimensionId()}] {
                        if (auto dim = ll::service::getLevel()->getDimension(dst.dim);
                            dim) {
                            auto& blockSource = dim->getBlockSourceFromMainChunkSource();
                            if (auto blockActor = blockSource.getBlockEntity(dst.pos);
                                blockActor) {
                                blockActor->refresh(blockSource);
                            }
                        }
                    },
                    1_tick
                );
            }
        })
    );
    listeners.emplace_back(bus.emplaceListener<PlayerSwingEvent>([this](
                                                                     PlayerSwingEvent& ev
                                                                 ) {
        auto hit =
            ev.self().traceRay(mod.getConfig().player_state.maximum_trace_length, false);
        if (!hit) {
            return;
        }
        if (hit.mIsHitLiquid && !ev.self().isImmersedInWater()) {
            hit.mBlock  = hit.mLiquid;
            hit.mFacing = hit.mLiquidFacing;
        }
        playerLeftClick(
            ev.self(),
            true,
            ev.self().getSelectedItem(),
            {hit.mBlock, ev.self().getDimensionId()},
            (FacingID)hit.mFacing
        );
    }));
    listeners.emplace_back(
        bus.emplaceListener<PlayerPlacingBlockEvent>([this](PlayerPlacingBlockEvent& ev) {
            ev.setCancelled(
                ClickState::Hold
                == playerRightClick(
                    ev.self(),
                    false,
                    ev.self().getSelectedItem(),
                    {ev.pos(), ev.self().getDimensionId()},
                    FacingID::Unknown
                )
            );
        })
    );
    listeners.emplace_back(bus.emplaceListener<PlayerInteractBlockEvent>(
        [this](PlayerInteractBlockEvent& ev) {
            ev.setCancelled(
                ClickState::Hold
                == playerRightClick(
                    ev.self(),
                    false,
                    ev.item(),
                    {ev.blockPos(), ev.self().getDimensionId()},
                    ev.face()
                )
            );
        }
    ));
    listeners.emplace_back(
        bus.emplaceListener<PlayerUseItemEvent>([this](PlayerUseItemEvent& ev) {
            auto hit = ev.self().traceRay(
                mod.getConfig().player_state.maximum_trace_length,
                false
            );
            if (!hit) {
                return;
            }
            if (hit.mIsHitLiquid && !ev.self().isImmersedInWater()) {
                hit.mBlock  = hit.mLiquid;
                hit.mFacing = hit.mLiquidFacing;
            }
            playerRightClick(
                ev.self(),
                true,
                ev.item(),
                {hit.mBlock, ev.self().getDimensionId()},
                (FacingID)hit.mFacing
            );
        })
    );
}
LocalContextManager::~LocalContextManager() {
    using namespace ll::event;
    for (auto& l : listeners) {
        EventBus::getInstance().removeListener(l);
    }
    playerStates.for_each([&](auto&& p) {
        if (p.second->temp) {
            return;
        }
        CompoundTag nbt;
        p.second->serialize(nbt);
        storagedState.set(p.first.asString(), nbt.toBinaryNbt());
    });
}
std::shared_ptr<LocalContext> LocalContextManager::get(mce::UUID const& uuid, bool temp) {
    std::shared_ptr<LocalContext> res;
    if (playerStates.if_contains(uuid, [&](auto&& p) { res = p.second; })) {
        return res;
    } else if (!temp) {
        if (auto nbt = storagedState.get(uuid.asString()); nbt) {
            res = std::make_shared<LocalContext>(uuid, temp);
            CompoundTag::fromBinaryNbt(*nbt).and_then([&](CompoundTag&& tag) {
                return res->deserialize(tag);
            });
            return playerStates.try_emplace(uuid, res).first->second;
        }
    }
    return res;
}
std::shared_ptr<LocalContext>
LocalContextManager::getOrCreate(mce::UUID const& uuid, bool temp) {
    std::shared_ptr<LocalContext> res;
    playerStates.lazy_emplace_l(
        uuid,
        [&](auto&& p) { res = p.second; },
        [&, this](auto&& ctor) {
            res = std::make_shared<LocalContext>(uuid, temp);
            if (!temp) {
                if (auto nbt = storagedState.get(uuid.asString()); nbt) {
                    CompoundTag::fromBinaryNbt(*nbt).and_then([&](CompoundTag&& tag) {
                        return res->deserialize(tag);
                    });
                }
            }
            ctor(uuid, res);
        }
    );
    return res;
}

std::shared_ptr<LocalContext> LocalContextManager::get(CommandOrigin const& o) {
    if (auto actor = o.getEntity(); actor && actor->isPlayer()) {
        return get(
            static_cast<Player*>(actor)->getUuid(),
            !static_cast<Player*>(actor)->isSimulated()
        );
    } else {
        return get({}, true);
    }
}

std::shared_ptr<LocalContext> LocalContextManager::getOrCreate(CommandOrigin const& o) {
    if (auto actor = o.getEntity(); actor && actor->isPlayer()) {
        return getOrCreate(
            static_cast<Player*>(actor)->getUuid(),
            !static_cast<Player*>(actor)->isSimulated()
        );
    } else {
        return getOrCreate({}, true);
    }
}

bool LocalContextManager::release(mce::UUID const& uuid) {
    return playerStates.erase_if(uuid, [&](auto&& p) {
        if (p.second->temp) {
            return true;
        }
        CompoundTag nbt;
        p.second->serialize(nbt);
        return storagedState.set(uuid.asString(), nbt.toBinaryNbt());
    });
}

void LocalContextManager::remove(mce::UUID const& uuid) {
    playerStates.erase(uuid);
    storagedState.del(uuid.asString());
}

// bool LocalContextManager::has(mce::UUID const& uuid, bool temp) {
//     if (temp) {
//         return playerStates.contains(uuid);
//     } else {
//         return playerStates.contains(uuid) || storagedState.get(uuid.asString());
//     }
// }

void LocalContextManager::removeTemps() {
    erase_if(playerStates, [](auto&& p) { return p.second->temp; });
}

LocalContextManager::ClickState LocalContextManager::playerLeftClick(
    Player&                  player,
    bool                     isLong,
    ItemStack const&         item,
    WithDim<BlockPos> const& dst,
    FacingID //  mFace TODO: brush
) {
    auto  data    = getOrCreate(player.getUuid(), player.isSimulated());
    auto& current = player.getLevel().getCurrentTick();
    bool  needDiscard{};
    if (current.tickID - data->lastLeftClick.load().tickID
        < mod.getConfig().player_state.minimum_response_tick) {
        needDiscard = true;
    }
    auto& itemName = item.getFullNameHash();
    if (data->config.wand == itemName) {
        if (isLong) {
            return ClickState::Pass;
        }
        data->lastLeftClick = current;
        if (!needDiscard) {
            data->setMainPos(dst);
        }
        return ClickState::Hold;
    }
    return ClickState::Pass;
}
LocalContextManager::ClickState LocalContextManager::playerRightClick(
    Player&                  player,
    bool                     isLong,
    ItemStack const&         item,
    WithDim<BlockPos> const& dst,
    FacingID // mFace TODO: brush
) {
    auto  data    = getOrCreate(player.getUuid(), player.isSimulated());
    auto& current = player.getLevel().getCurrentTick();
    bool  needDiscard{};
    if (current.tickID - data->lastRightClick.load().tickID
        < mod.getConfig().player_state.minimum_response_tick) {
        needDiscard = true;
    }
    auto& itemName = item.getFullNameHash();
    if (data->config.wand == itemName) {
        if (isLong) {
            return ClickState::Pass;
        }
        data->lastRightClick = current;
        if (!needDiscard) {
            data->setOffPos(dst);
        }
        return ClickState::Hold;
    }
    return ClickState::Pass;
}

} // namespace we
