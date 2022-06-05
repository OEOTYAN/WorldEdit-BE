//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#include "WorldEdit.h"
#include "HookAPI.h"
#include <MC/Block.hpp>
#include <MC/BlockLegacy.hpp>
#include <MC/ServerNetworkHandler.hpp>
#include <MC/NetworkPacketEventCoordinator.hpp>
#include <MC/PacketHeader.hpp>
#include <MC/Packet.hpp>
#include <MC/NetworkIdentifier.hpp>
#include <MC/PlayerActionPacket.hpp>
#include <fstream>

//?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@AEBVPlayerActionPacket@@@Z
// THook(void,
//       "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@"
//       "AEBVPlayerActionPacket@@@Z",
//       ServerNetworkHandler* serverNetworkHandler,
//       NetworkIdentifier const& networkIdentifier,
//       PlayerActionPacket const& playerActionPacket) {
//     std::cout
//         <<
//         const_cast<PlayerActionPacket&>(playerActionPacket).toDebugString()
//         << std::endl;
//     return original(serverNetworkHandler, networkIdentifier,
//                     playerActionPacket);
// }
//?sendPacketReceivedFrom@NetworkPacketEventCoordinator@@QEAAXAEBVPacketHeader@@AEBVPacket@@@Z
// THook(void,
//       "?sendPacketReceivedFrom@NetworkPacketEventCoordinator@@"
//       "QEAAXAEBVPacketHeader@@AEBVPacket@@@Z",
//       NetworkPacketEventCoordinator* networkPacketEventCoordinator,
//       PacketHeader const& packetHeader,
//       Packet const& packet) {
//     std::cout << fmt::format("{}({})->{}", packet.getName(), packet.getId(),
//                              packet.clientSubId)
//               << std::endl;
//     return original(networkPacketEventCoordinator, packetHeader, packet);
// }
THook(void,
      "?setRuntimeId@Block@@IEBAXAEBI@Z",
      Block* block,
      unsigned int const& id) {
    auto& blockName = worldedit::getBlockNameMap();
    auto& blockId = worldedit::getBlockIdMap();
    auto blockid = block->getId();
    blockName[blockid] = block->getTypeName();
    blockId[block->getTypeName()] = blockid;
    return original(block, id);
}

namespace worldedit {
    WE& getMod() {
        static WE t;
        return t;
    }

    std::unordered_map<int, std::string>& getBlockNameMap() {
        static std::unordered_map<int, std::string> blockName;
        return blockName;
    }

    std::unordered_map<std::string, int>& getBlockIdMap() {
        static std::unordered_map<std::string, int> blockId;
        return blockId;
    }
    std::string getBlockName(int id) {
        auto& blockName = worldedit::getBlockNameMap();
        if (blockName.find(id) != blockName.end())
            return blockName[id];
        return "minecraft:air";
    }
    int getBlockId(const std::string& name) {
        auto& blockId = worldedit::getBlockIdMap();
        if (blockId.find(name) != blockId.end())
            return blockId[name];
        return 0;
    }
    void WE::renderMVpos() {
        for (auto& PosPair : playerMainPosMap) {
            if (PosPair.second.second.first <= 0) {
                PosPair.second.second.first = 40;
                worldedit::spawnCuboidParticle(
                    AABB(
                        Vec3(PosPair.second.first) - Vec3(0.07f, 0.07f, 0.07f),
                        Vec3(PosPair.second.first) + Vec3(1.07f, 1.07f, 1.07f)),
                    GRAPHIC_COLOR::RED, PosPair.second.second.second);
            }
            PosPair.second.second.first--;
        }
        for (auto& PosPair : playerVicePosMap) {
            if (PosPair.second.second.first <= 0) {
                PosPair.second.second.first = 40;
                worldedit::spawnCuboidParticle(
                    AABB(
                        Vec3(PosPair.second.first) - Vec3(0.06f, 0.06f, 0.06f),
                        Vec3(PosPair.second.first) + Vec3(1.06f, 1.06f, 1.06f)),
                    GRAPHIC_COLOR::BLUE, PosPair.second.second.second);
            }
            PosPair.second.second.first--;
        }
    }

    Clipboard* WE::getPlayerNextHistory(std::string xuid) {
        if (playerHistoryMap.find(xuid) == playerHistoryMap.end()) {
            playerHistoryMap[xuid].first.resize(maxHistoryLength);
            playerHistoryMap[xuid].second.first = 0;
        } else {
            playerHistoryMap[xuid].second.first += 1;
            playerHistoryMap[xuid].second.first %= maxHistoryLength;
        }
        playerHistoryMap[xuid].second.second =
            playerHistoryMap[xuid].second.first;
        return &playerHistoryMap[xuid]
                    .first[playerHistoryMap[xuid].second.first];
    }

    std::pair<Clipboard*, int> WE::getPlayerUndoHistory(std::string xuid) {
        std::pair<Clipboard*, int> result = {nullptr, -1};
        if (playerHistoryMap.find(xuid) != playerHistoryMap.end()) {
            int maximum =
                (playerHistoryMap[xuid].second.second + 1) % maxHistoryLength;
            int current = playerHistoryMap[xuid].second.first;
            if (maximum == current) {
                result.second = 0;
                return result;
            }
            result.first = &playerHistoryMap[xuid].first[current];
            if (result.first->used == false) {
                result.second = 0;
                return result;
            }
            result.second = 1;
            current -= 1;
            if (current < 0) {
                current += maxHistoryLength;
            }
            playerHistoryMap[xuid].second.first = current;
            return result;
        }
        return result;
    }

    std::pair<Clipboard*, int> WE::getPlayerRedoHistory(std::string xuid) {
        std::pair<Clipboard*, int> result = {nullptr, -1};
        if (playerHistoryMap.find(xuid) != playerHistoryMap.end()) {
            int maximum =
                (playerHistoryMap[xuid].second.second + 1) % maxHistoryLength;
            int current = playerHistoryMap[xuid].second.first + 1;
            current %= maxHistoryLength;
            if (maximum == current) {
                result.second = 0;
                return result;
            }
            result.first = &playerHistoryMap[xuid].first[current];
            if (result.first->used == false) {
                result.second = 0;
                return result;
            }
            result.second = 1;
            playerHistoryMap[xuid].second.first = current;
            return result;
        }
        return result;
    }
}  // namespace worldedit