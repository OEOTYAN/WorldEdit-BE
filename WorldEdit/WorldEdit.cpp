//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#include "WorldEdit.h"
#include "HookAPI.h"
#include "ParticleAPI.h"
#include "particle/Graphics.h"
#include "utils/StringTool.h"
#include "eval/Eval.h"
#include <mc/Block.hpp>
#include <mc/Level.hpp>
#include <mc/BlockLegacy.hpp>
#include <mc/Packet.hpp>
#include <mc/Player.hpp>
#include <fstream>
#include "Version.h"
#include "Nlohmann/json.hpp"

THook(void, "?setRuntimeId@Block@@IEBAXAEBI@Z", Block* block, unsigned int const& id) {
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

    Logger& logger() {
        static Logger l(PLUGIN_NAME);
#if PLUGIN_VERSION_STATUS == PLUGIN_VERSION_DEV
        l.consoleLevel = 8;
#endif
        return l;
    }

    void clearBlockEntity(class BlockActor* be) {
        void* unknown = VirtualCall<void*>(be, 256);
        //  logger().info("{}", (long long)unknown);
        if (unknown != nullptr) {
            VirtualCall<void>(unknown, 96);
        }
    }

    phmap::flat_hash_map<std::string, class PlayerData>& getPlayersDataMap() {
        static phmap::flat_hash_map<std::string, class PlayerData> data;
        return data;
    }

    class PlayerData& getPlayersData(std::string xuid) {
        auto& playerDataMap = getPlayersDataMap();
        if (playerDataMap.find(xuid) == playerDataMap.end()) {
            playerDataMap[xuid] = PlayerData(Global<Level>->getPlayer(xuid));
        }
        return playerDataMap[xuid];
    }

    phmap::flat_hash_map<mce::Color, Block*>& getColorBlockMap() {
        static phmap::flat_hash_map<mce::Color, Block*> colorBlockMap;
        return colorBlockMap;
    }

    phmap::flat_hash_map<Block*, mce::Color>& getBlockColorMap() {
        static phmap::flat_hash_map<Block*, mce::Color> blockColorMap;
        return blockColorMap;
    }

    phmap::flat_hash_map<int, std::string>& getBlockNameMap() {
        static phmap::flat_hash_map<int, std::string> blockName;
        return blockName;
    }

    phmap::flat_hash_map<std::string, int>& getBlockIdMap() {
        static phmap::flat_hash_map<std::string, int> blockId;
        return blockId;
    }

    phmap::flat_hash_map<std::string, Block*>& getJavaBlockMap() {
        static phmap::flat_hash_map<std::string, Block*> javaBlockMap;
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

    bool isBEBlock(const std::string& s) {
        auto& blockId = worldedit::getBlockIdMap();
        return blockId.find(s) != blockId.end();
    }

    bool isJEBlock(const std::string& s) {
        auto& blockId = worldedit::getJavaBlockMap();
        return blockId.find(s) != blockId.end();
    }

    Block* tryGetBlockFromAllVersion(const std::string& name) {
        if (isJEBlock(name)) {
            return getJavaBlockMap()[name];
        } else if (isBEBlock(name)) {
            return Block::create(name, 0);
        }
        return nullptr;
    }

    void javaBlockMapInit() {
        nlohmann::json blockList;
        std::ifstream i(WE_DIR + "mappings/blocks.json");
        i >> blockList;

        for (auto& b : blockList.items()) {
            std::string key = b.key();
            Block* block = nullptr;
            if (b.value().contains("bedrock_states")) {
                std::string snbt = "{\"name\":\"";
                snbt += b.value()["bedrock_identifier"];
                snbt += "\",\"states\":";
                std::string states = b.value()["bedrock_states"].dump();
                stringReplace(states, ":false", ":0b");
                stringReplace(states, ":true", ":1b");
                snbt += states;
                snbt += "}";
                block = Block::create(CompoundTag::fromSNBT(snbt).get());
            } else {
                block = Block::create(b.value()["bedrock_identifier"], 0);
            }
            if (block != nullptr) {
                getJavaBlockMap()[key] = block;
            }
        }
    }

    void blockColorMapInit() {
        nlohmann::json blockColorList;
        std::ifstream i(WE_DIR + "mappings/block_colors.json");
        i >> blockColorList;
        for (auto& blockColor : blockColorList["blocks"]) {
            Block* block = tryGetBlockFromAllVersion(blockColor["name"]);
            if (block != nullptr) {
                auto color = mce::Color(
                    static_cast<double>(blockColor["colour"]["r"]), static_cast<double>(blockColor["colour"]["g"]),
                    static_cast<double>(blockColor["colour"]["b"]), static_cast<double>(blockColor["colour"]["a"]));
                getColorBlockMap()[color] = block;
                getBlockColorMap()[block] = color;
            }
            //  else {
            //     std::cout << blockColor["name"] << std::endl;
            // }
        }
    }

    void setFunction(phmap::flat_hash_map<::std::string, double>& variables,
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