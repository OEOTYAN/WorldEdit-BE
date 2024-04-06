#include "tool.h"

#include "WorldEdit.h"
#include "mc/BlockInstance.hpp"
#include "store/Patterns.h"
#include "utils/StringTool.h"
namespace we {

////////
long long FarWand::lset(Player* player, class BlockInstance blockInstance) {
    auto  xuid       = player->getXuid();
    auto& playerData = getPlayersData(xuid);
    playerData.changeMainPos(blockInstance);
    return -2;
}
long long FarWand::set(Player* player, class BlockInstance blockInstance) {
    auto  xuid       = player->getXuid();
    auto& playerData = getPlayersData(xuid);
    playerData.changeVicePos(blockInstance);
    return -2;
}

////////
long long AirWand::lset(Player* player, class BlockInstance blockInstance) {
    auto  xuid       = player->getXuid();
    auto& playerData = getPlayersData(xuid);
    auto  bs         = Level::getBlockSource(player);
    playerData.changeMainPos(bs->getBlockInstance(player->getBlockPos()));
    return -2;
}
long long AirWand::set(Player* player, class BlockInstance blockInstance) {
    auto  xuid       = player->getXuid();
    auto& playerData = getPlayersData(xuid);
    auto  bs         = Level::getBlockSource(player);
    playerData.changeVicePos(bs->getBlockInstance(player->getBlockPos()));
    return -2;
}

////////
long long InfoTool::set(Player* player, class BlockInstance blockInstance) {
    if (blockInstance == BlockInstance::Null) {
        return -2;
    }
    auto block   = blockInstance.getBlock();
    auto exblock = &const_cast<Block&>(
        blockInstance.getBlockSource()->getExtraBlock(blockInstance.getPosition())
    );
    std::cout << "block: (" << getBlockId(block->getTypeName()) << ":"
              << block->getTileData() << ") " << block->getRuntimeId() << "\n"
              << block->getNbt()->toPrettySNBT() << std::endl;
    player->sendText(
        "block: (" + fto_string(getBlockId(block->getTypeName())) + ":"
        + fto_string(block->getTileData()) + ") " + fto_string(block->getRuntimeId())
        + "\n" + block->getNbt()->toPrettySNBT(true)
    );
    if (exblock != BedrockBlocks::mAir) {
        std::cout << "exBlock: (" << getBlockId(exblock->getTypeName()) << ":"
                  << exblock->getTileData() << ") " << exblock->getRuntimeId() << "\n"
                  << exblock->getNbt()->toPrettySNBT() << std::endl;
        player->sendText(
            "exBlock: (" + fto_string(getBlockId(exblock->getTypeName())) + ":"
            + fto_string(exblock->getTileData()) + ") "
            + fto_string(exblock->getRuntimeId()) + "\n"
            + exblock->getNbt()->toPrettySNBT(true)
        );
    }
    if (blockInstance.hasBlockEntity()) {
        auto be = blockInstance.getBlockEntity();
        if (be != nullptr) {
            std::cout << "blockEntity:\n" << be->getNbt()->toPrettySNBT() << std::endl;
            player->sendText("blockEntity:\n" + be->getNbt()->toPrettySNBT(true));
        } else {
            std::cout << "blockEntity: null" << std::endl;
            player->sendText("blockEntity: null");
        }
    }
    return -2;
}

////////
long long CyclerTool::lset(Player* player, class BlockInstance blockInstance) {
    if (blockInstance == BlockInstance::Null) {
        return -2;
    }
    return -2;
}
long long CyclerTool::set(Player* player, class BlockInstance blockInstance) {
    if (blockInstance == BlockInstance::Null) {
        return -2;
    }
    return -2;
}

////////
long long RepTool::lset(Player* player, class BlockInstance blockInstance) {
    if (blockInstance == BlockInstance::Null) {
        return -2;
    }
    blockSet = std::move(BlockNBTSet(blockInstance));
    return -2;
}
long long RepTool::set(Player* player, class BlockInstance blockInstance) {
    if (blockInstance == BlockInstance::Null) {
        return -2;
    }
    auto blockSource = blockInstance.getBlockSource();
    auto bpos        = blockInstance.getPosition();

    auto  xuid       = player->getXuid();
    auto& playerData = getPlayersData(xuid);

    EvalFunctions f;
    f.setbs(blockSource);
    phmap::flat_hash_map<std::string, double> variables;

    if (playerData.maxHistoryLength > 0) {
        auto& history          = playerData.getNextHistory();
        history                = std::move(Clipboard(BlockPos(0, 0, 0)));
        history.playerRelPos.x = blockInstance.getDimensionId();
        history.playerPos      = bpos;
        history.storeBlock(blockInstance, BlockPos(0, 0, 0));
    }
    f.setPos(bpos);
    auto playerPos  = player->getPosition();
    auto playerRot  = player->getRotation();
    variables["rx"] = bpos.x;
    variables["ry"] = bpos.y;
    variables["rz"] = bpos.z;
    variables["px"] = playerPos.x;
    variables["py"] = playerPos.y;
    variables["pz"] = playerPos.z;
    variables["pp"] = playerRot.x;
    variables["pt"] = playerRot.y;
    variables["ox"] = bpos.x - playerPos.x;
    variables["oy"] = bpos.y - playerPos.y;
    variables["oz"] = bpos.z - playerPos.z;
    maskFunc(f, variables, [&]() mutable {
        blockSet.setBlock(bpos, blockSource, playerData, f, variables);
    });

    return -2;
}

long long FloodFillTool::set(Player* player, class BlockInstance blockInstance) {
    if (blockInstance == BlockInstance::Null) {
        return -2;
    }
    std::queue<BlockPos> q;
    auto                 pos0 = blockInstance.getPosition();
    q.push(pos0);

    auto         xuid        = player->getXuid();
    auto&        playerData  = getPlayersData(xuid);
    auto         blockSource = blockInstance.getBlockSource();
    Block const* block       = blockInstance.getBlock();
    Block const* exBlock     = &blockSource->getExtraBlock(pos0);

    EvalFunctions f;
    f.setbs(blockSource);
    phmap::flat_hash_map<std::string, double> variables;
    auto                                      playerPos = player->getPosition();
    auto                                      playerRot = player->getRotation();
    variables["px"]                                     = playerPos.x;
    variables["py"]                                     = playerPos.y;
    variables["pz"]                                     = playerPos.z;
    variables["pp"]                                     = playerRot.x;
    variables["pt"]                                     = playerRot.y;
    phmap::flat_hash_set<BlockPos> s;
    s.insert(pos0);

    BoundingBox boundingBox = BoundingBox(pos0, pos0);

    while (!q.empty()) {
        auto pos1 = q.front();
        q.pop();
        boundingBox.merge(pos1);
        if (needEdge) {
            BoundingBox(pos1 - 1, pos1 + 1)
                .forEachBlockInBox([&](BlockPos const& tmpPos) {
                    if (tmpPos != pos1 && pos0.distanceTo(tmpPos) <= 0.5 + size
                        && (&blockSource->getBlock(tmpPos) == block
                            && &blockSource->getExtraBlock(tmpPos) == exBlock)
                        && s.find(tmpPos) == s.end()) {
                        f.setPos(tmpPos);
                        variables["x"]  = static_cast<double>(tmpPos.x - pos0.x) / size;
                        variables["y"]  = static_cast<double>(tmpPos.y - pos0.y) / size;
                        variables["z"]  = static_cast<double>(tmpPos.z - pos0.z) / size;
                        variables["rx"] = tmpPos.x;
                        variables["ry"] = tmpPos.y;
                        variables["rz"] = tmpPos.z;
                        variables["cx"] = tmpPos.x - pos0.x;
                        variables["cy"] = tmpPos.y - pos0.y;
                        variables["cz"] = tmpPos.z - pos0.z;
                        variables["ox"] = tmpPos.x - playerPos.x;
                        variables["oy"] = tmpPos.y - playerPos.y;
                        variables["oz"] = tmpPos.z - playerPos.z;
                        maskFunc(f, variables, [&]() mutable {
                            q.push(tmpPos);
                            s.insert(tmpPos);
                        });
                    }
                });
        } else {
            for (auto& tmpPos : pos1.getNeighbors()) {
                if (pos0.distanceTo(tmpPos) <= 0.5 + size
                    && (&blockSource->getBlock(tmpPos) == block
                        && &blockSource->getExtraBlock(tmpPos) == exBlock)
                    && s.find(tmpPos) == s.end()) {
                    f.setPos(tmpPos);
                    variables["x"]  = static_cast<double>(tmpPos.x - pos0.x) / size;
                    variables["y"]  = static_cast<double>(tmpPos.y - pos0.y) / size;
                    variables["z"]  = static_cast<double>(tmpPos.z - pos0.z) / size;
                    variables["rx"] = tmpPos.x;
                    variables["ry"] = tmpPos.y;
                    variables["rz"] = tmpPos.z;
                    variables["cx"] = tmpPos.x - pos0.x;
                    variables["cy"] = tmpPos.y - pos0.y;
                    variables["cz"] = tmpPos.z - pos0.z;
                    variables["ox"] = tmpPos.x - playerPos.x;
                    variables["oy"] = tmpPos.y - playerPos.y;
                    variables["oz"] = tmpPos.z - playerPos.z;
                    maskFunc(f, variables, [&]() mutable {
                        q.push(tmpPos);
                        s.insert(tmpPos);
                    });
                }
            }
        }
    }

    if (playerData.maxHistoryLength > 0) {
        auto& history          = playerData.getNextHistory();
        history                = std::move(Clipboard(boundingBox.max - boundingBox.min));
        history.playerRelPos.x = blockInstance.getDimensionId();
        history.playerPos      = boundingBox.min;
        for (auto& pos1 : s) {
            auto localPos = pos1 - boundingBox.min;
            auto bi       = blockSource->getBlockInstance(pos1);
            history.storeBlock(bi, localPos);
        }
    }

    for (auto& pos1 : s) {
        f.setPos(pos1);
        variables["x"]  = static_cast<double>(pos1.x - pos0.x) / size;
        variables["y"]  = static_cast<double>(pos1.y - pos0.y) / size;
        variables["z"]  = static_cast<double>(pos1.z - pos0.z) / size;
        variables["rx"] = pos1.x;
        variables["ry"] = pos1.y;
        variables["rz"] = pos1.z;
        variables["cx"] = pos1.x - pos0.x;
        variables["cy"] = pos1.y - pos0.y;
        variables["cz"] = pos1.z - pos0.z;
        variables["ox"] = pos1.x - playerPos.x;
        variables["oy"] = pos1.y - playerPos.y;
        variables["oz"] = pos1.z - playerPos.z;
        pattern->setBlock(variables, f, blockSource, pos1);
    }

    return -2;
}

////////
long long TreeTool::set(Player* player, class BlockInstance blockInstance) {
    // if (blockInstance == BlockInstance::Null) {
    //
    return -2;
    // }
    // auto bs = Level::getBlockSource(player);
    // auto pos = blockInstance.getPosition();
    // setBlockSimple(bs, pos,
    // StaticVanillaBlocks::mGrass); pos.y += 1;
    // setBlockSimple(bs, pos,
    // StaticVanillaBlocks::mSapling); auto& legacyBlock
    // = blockInstance.getBlock()->getLegacyBlock(); return
    // ((Sapling&)legacyBlock)._growTree(*bs, pos,
    // Global<Level>->getRandom(), false);

    return -2;
}

////////
long long DelTreeTool::set(Player* player, class BlockInstance blockInstance) {
    if (blockInstance == BlockInstance::Null) {
        return -2;
    }

    return -2;
}

} // namespace we