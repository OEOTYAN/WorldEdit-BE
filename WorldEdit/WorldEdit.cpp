//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#include "WorldEdit.h"
#include "HookAPI.h"
#include "ParticleAPI.h"
#include "particle/Graphics.h"
#include "eval/Eval.h"
#include <MC/Block.hpp>
#include <MC/Level.hpp>
#include <MC/BlockLegacy.hpp>
#include <MC/ServerNetworkHandler.hpp>
#include <MC/NetworkPacketEventCoordinator.hpp>
#include <MC/PacketHeader.hpp>
#include <MC/Packet.hpp>
#include <MC/NetworkIdentifier.hpp>
#include <MC/PlayerActionPacket.hpp>
#include <MC/Player.hpp>
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

// std::unordered_map<std::string, std::string>& getBlockColorssMap() {
//     static std::unordered_map<std::string, std::string> sb;
//     return sb;
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
    // auto color = block->getLegacyBlock().getMapColor().toHexString();
    // if (getBlockColorssMap().find(color) != getBlockColorssMap().end()) {
    //     getBlockColorssMap()[color] += ",\n" + block->getNbt()->toSNBT(0,
    //     SnbtFormat::Minimize);
    // } else {
    //     getBlockColorssMap()[color] = block->getNbt()->toSNBT(0,
    //     SnbtFormat::Minimize);
    // }
    return original(block, id);
}

namespace worldedit {

    std::unordered_map<std::string, class PlayerData>& getPlayersDataMap() {
        static std::unordered_map<std::string, class PlayerData> data;
        return data;
    }

    class PlayerData& getPlayersData(std::string xuid) {
        auto& playerDataMap = getPlayersDataMap();
        if (playerDataMap.find(xuid) == playerDataMap.end()) {
            playerDataMap[xuid] = PlayerData(Global<Level>->getPlayer(xuid));
        }
        return playerDataMap[xuid];
    }

    std::unordered_map<mce::Color, int>& getBlockColorMap() {
        static std::unordered_map<mce::Color, int> blockColorMap;

        static bool lized = false;
        if (!lized) {
            blockColorMap[mce::Color(208, 214, 215)] = 0;
            blockColorMap[mce::Color(224, 96, 0)] = 1;
            blockColorMap[mce::Color(170, 45, 160)] = 2;
            blockColorMap[mce::Color(30, 138, 200)] = 3;
            blockColorMap[mce::Color(240, 175, 13)] = 4;
            blockColorMap[mce::Color(93, 168, 16)] = 5;
            blockColorMap[mce::Color(215, 102, 145)] = 6;
            blockColorMap[mce::Color(52, 56, 60)] = 7;
            blockColorMap[mce::Color(125, 125, 125)] = 8;
            blockColorMap[mce::Color(13, 119, 136)] = 9;
            blockColorMap[mce::Color(100, 25, 157)] = 10;
            blockColorMap[mce::Color(40, 42, 144)] = 11;
            blockColorMap[mce::Color(96, 57, 25)] = 12;
            blockColorMap[mce::Color(72, 91, 31)] = 13;
            blockColorMap[mce::Color(144, 30, 30)] = 14;
            blockColorMap[mce::Color(2, 3, 7)] = 15;
            lized = true;
        }
        return blockColorMap;
    }

    std::unordered_map<int, std::string>& getBlockNameMap() {
        static std::unordered_map<int, std::string> blockName;
        return blockName;
    }

    std::unordered_map<std::string, int>& getBlockIdMap() {
        static std::unordered_map<std::string, int> blockId;
        return blockId;
    }

    std::unordered_map<std::string, Block*>& getJavaBlockMap() {
        static std::unordered_map<std::string, Block*> javaBlockMap;
        return javaBlockMap;
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

bool isBEBlock(const std::string& s){
        auto& blockId = worldedit::getBlockIdMap();
    return blockId.find(s) != blockId.end();
}

bool isJEBlock(const std::string& s){
        auto& blockId = worldedit::getJavaBlockMap();
    return blockId.find(s) != blockId.end();
}

    void setFunction(std::unordered_map<::std::string, double>& variables,
                     EvalFunctions& funcs,
                     const BoundingBox& boundingBox,
                     const Vec3& playerPos,
                     const BlockPos& pos,
                     const Vec3& center) {
        funcs.setPos(pos);
        double lengthx = (boundingBox.max.x - boundingBox.min.x) * 0.5;
        double lengthy = (boundingBox.max.y - boundingBox.min.y) * 0.5;
        double lengthz = (boundingBox.max.z - boundingBox.min.z) * 0.5;
        double centerx = (boundingBox.max.x + boundingBox.min.x) * 0.5;
        double centery = (boundingBox.max.y + boundingBox.min.y) * 0.5;
        double centerz = (boundingBox.max.z + boundingBox.min.z) * 0.5;
        variables["x"] = (pos.x - centerx) / lengthx;
        variables["y"] = (pos.y - centery) / lengthy;
        variables["z"] = (pos.z - centerz) / lengthz;
        variables["rx"] = pos.x;
        variables["ry"] = pos.y;
        variables["rz"] = pos.z;
        variables["ox"] = pos.x - playerPos.x;
        variables["oy"] = pos.y - playerPos.y;
        variables["oz"] = pos.z - playerPos.z;
        variables["cx"] = pos.x - floor(center.x);
        variables["cy"] = pos.y - floor(center.y);
        variables["cz"] = pos.z - floor(center.z);
    }

}  // namespace worldedit