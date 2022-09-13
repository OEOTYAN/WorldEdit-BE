//
// Created by OEOTYAN on 2022/06/10.
//
#include "tool.h"
#include "region/ChangeRegion.hpp"
#include "string/StringTool.h"
#include "WorldEdit.h"
#include "MC/Sapling.hpp"
#include "store/BlockPattern.hpp"
namespace worldedit {

    bool Tool::leftClick(Player* player, BlockInstance& blockInstance) {
        return false;
    }
    bool Tool::rightClick(Player* player, BlockInstance& blockInstance) {
        return false;
    }

    ////////
    bool FarWand::leftClick(Player* player, BlockInstance& blockInstance) {
        changeMainPos(player, blockInstance);
        return true;
    }
    bool FarWand::rightClick(Player* player, BlockInstance& blockInstance) {
        changeVicePos(player, blockInstance);
        return true;
    }

    ////////
    bool AirWand::leftClick(Player* player, BlockInstance& blockInstance) {
        auto bs = Level::getBlockSource(player);
        changeMainPos(player, bs->getBlockInstance((player->getPosition() - Vec3(0.0, 1.62, 0.0)).toBlockPos()));
        return true;
    }
    bool AirWand::rightClick(Player* player, BlockInstance& blockInstance) {
        auto bs = Level::getBlockSource(player);
        changeVicePos(player, bs->getBlockInstance((player->getPosition() - Vec3(0.0, 1.62, 0.0)).toBlockPos()));
        return true;
    }

    ////////
    bool InfoTool::rightClick(Player* player, BlockInstance& blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return false;
        }
        auto block = blockInstance.getBlock();
        auto exblock = &const_cast<Block&>(blockInstance.getBlockSource()->getExtraBlock(blockInstance.getPosition()));
        std::cout << "block: (" << getBlockId(block->getTypeName()) << ":" << block->getTileData() << ") "
                  << block->getRuntimeId() << "\n"
                  << block->getNbt()->toPrettySNBT() << std::endl;
        player->sendText("block: (" + fto_string(getBlockId(block->getTypeName())) + ":" +
                         fto_string(block->getTileData()) + ") " + fto_string(block->getRuntimeId()) + "\n" +
                         block->getNbt()->toPrettySNBT(true));
        if (exblock != BedrockBlocks::mAir) {
            std::cout << "exBlock: (" << getBlockId(exblock->getTypeName()) << ":" << exblock->getTileData() << ") "
                      << exblock->getRuntimeId() << "\n"
                      << exblock->getNbt()->toPrettySNBT() << std::endl;
            player->sendText("exBlock: (" + fto_string(getBlockId(exblock->getTypeName())) + ":" +
                             fto_string(exblock->getTileData()) + ") " + fto_string(exblock->getRuntimeId()) + "\n" +
                             exblock->getNbt()->toPrettySNBT(true));
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
        return true;
    }

    ////////
    bool CyclerTool::leftClick(Player* player, BlockInstance& blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return false;
        }
        return false;
    }
    bool CyclerTool::rightClick(Player* player, BlockInstance& blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return false;
        }
        return true;
    }

    ////////
    bool RepTool::leftClick(Player* player, BlockInstance& blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return false;
        }
        blockSet = BlockNBTSet(blockInstance);
        return true;
    }
    bool RepTool::rightClick(Player* player, BlockInstance& blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return false;
        }
        auto blockSource = blockInstance.getBlockSource();
        auto bpos = blockInstance.getPosition();
        auto& mod = worldedit::getMod();
        auto xuid = player->getXuid();

        INNERIZE_GMASK

        EvalFunctions f;
        f.setbs(blockSource);
        std::unordered_map<std::string, double> variables;

        if (mod.maxHistoryLength > 0) {
            auto history = mod.getPlayerNextHistory(xuid);
            *history = Clipboard(BlockPos(0, 0, 0));
            history->playerRelPos.x = blockInstance.getDimensionId();
            history->playerPos = bpos;
            history->storeBlock(blockInstance, BlockPos(0, 0, 0));
        }
        f.setPos(bpos);
        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
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
        gMaskLambda(f, variables, [&]() mutable { blockSet.setBlock(bpos, blockSource); });

        return false;
    }

    ////////
    FloodFillTool::~FloodFillTool() {
        delete pattern;
    }
    bool FloodFillTool::rightClick(Player* player, BlockInstance& blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return false;
        }
        std::queue<BlockPos> q;
        auto pos0 = blockInstance.getPosition();
        q.push(pos0);
        auto& mod = worldedit::getMod();
        auto xuid = player->getXuid();
        auto blockSource = blockInstance.getBlockSource();
        const Block* block = blockInstance.getBlock();
        const Block* exBlock = &blockSource->getExtraBlock(pos0);

        INNERIZE_GMASK

        EvalFunctions f;
        f.setbs(blockSource);
        std::unordered_map<std::string, double> variables;
        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        variables["px"] = playerPos.x;
        variables["py"] = playerPos.y;
        variables["pz"] = playerPos.z;
        variables["pp"] = playerRot.x;
        variables["pt"] = playerRot.y;
        std::unordered_set<BlockPos> s;
        s.insert(pos0);

        BoundingBox boundingBox = BoundingBox(pos0, pos0);

        while (!q.empty()) {
            auto pos1 = q.front();
            q.pop();
            boundingBox = boundingBox.merge(pos1);
            if (needEdge) {
                BoundingBox(pos1 - 1, pos1 + 1).forEachBlockInBox([&](const BlockPos& tmpPos) {
                    if (tmpPos != pos1 && pos0.distanceTo(tmpPos) <= 0.5 + radius &&
                        (&blockSource->getBlock(tmpPos) == block && &blockSource->getExtraBlock(tmpPos) == exBlock) &&
                        s.find(tmpPos) == s.end()) {
                        f.setPos(tmpPos);
                        variables["rx"] = tmpPos.x;
                        variables["ry"] = tmpPos.y;
                        variables["rz"] = tmpPos.z;
                        variables["cx"] = tmpPos.x-pos0.x;
                        variables["cy"] = tmpPos.y-pos0.y;
                        variables["cz"] = tmpPos.z-pos0.z;
                        variables["ox"] = tmpPos.x - playerPos.x;
                        variables["oy"] = tmpPos.y - playerPos.y;
                        variables["oz"] = tmpPos.z - playerPos.z;
                        gMaskLambda(f, variables, [&]() mutable {
                            q.push(tmpPos);
                            s.insert(tmpPos);
                        });
                    }
                });
            } else {
                for (auto& tmpPos : pos1.getNeighbors()) {
                    if (pos0.distanceTo(tmpPos) <= 0.5 + radius &&
                        (&blockSource->getBlock(tmpPos) == block && &blockSource->getExtraBlock(tmpPos) == exBlock) &&
                        s.find(tmpPos) == s.end()) {
                        f.setPos(tmpPos);
                        variables["rx"] = tmpPos.x;
                        variables["ry"] = tmpPos.y;
                        variables["rz"] = tmpPos.z;
                        variables["cx"] = tmpPos.x - pos0.x;
                        variables["cy"] = tmpPos.y - pos0.y;
                        variables["cz"] = tmpPos.z - pos0.z;
                        variables["ox"] = tmpPos.x - playerPos.x;
                        variables["oy"] = tmpPos.y - playerPos.y;
                        variables["oz"] = tmpPos.z - playerPos.z;
                        gMaskLambda(f, variables, [&]() mutable {
                            q.push(tmpPos);
                            s.insert(tmpPos);
                        });
                    }
                }
            }
        }

        worldedit::Clipboard* history = nullptr;

        if (mod.maxHistoryLength > 0) {
            history = mod.getPlayerNextHistory(xuid);
            *history = Clipboard(boundingBox.max - boundingBox.min);
            history->playerRelPos.x = blockInstance.getDimensionId();
            history->playerPos = boundingBox.min;
        }

        for (auto& pos1 : s) {
            if (history != nullptr) {
                auto localPos = pos1 - boundingBox.min;
                auto bi = blockSource->getBlockInstance(pos1);
                history->storeBlock(bi, localPos);
            }
            pattern->setBlock(variables, f, blockSource, pos1);
        }

        return false;
    }

    ////////
    bool TreeTool::rightClick(Player* player, BlockInstance& blockInstance) {
        // if (blockInstance == BlockInstance::Null) {
        //     return false;
        // }
        // auto bs = Level::getBlockSource(player);
        // auto pos = blockInstance.getPosition();
        // setBlockSimple(bs, pos, const_cast<Block*>(StaticVanillaBlocks::mGrass));
        // pos.y += 1;
        // setBlockSimple(bs, pos, const_cast<Block*>(StaticVanillaBlocks::mSapling));
        // auto& legacyBlock = blockInstance.getBlock()->getLegacyBlock();
        // return ((Sapling&)legacyBlock)._growTree(*bs, pos, Global<Level>->getRandom(), false);
        return false;
    }

    ////////
    bool DelTreeTool::rightClick(Player* player, BlockInstance& blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return false;
        }
        return false;
    }

}  // namespace worldedit