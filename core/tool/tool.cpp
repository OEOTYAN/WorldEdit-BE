//
// Created by OEOTYAN on 2022/06/10.
//
#include "tool.h"
#include "region/ChangeRegion.hpp"
#include "string/StringTool.h"
#include "WorldEdit.h"
#include "MC/Sapling.hpp"
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
        changeMainPos(player, bs->getBlockInstance((player->getPosition() - Vec3(0.0, 1.79, 0.0)).toBlockPos()));
        return true;
    }
    bool AirWand::rightClick(Player* player, BlockInstance& blockInstance) {
        auto bs = Level::getBlockSource(player);
        changeVicePos(player, bs->getBlockInstance((player->getPosition() - Vec3(0.0, 1.79, 0.0)).toBlockPos()));
        return true;
    }

    ////////
    bool InfoTool::rightClick(Player* player, BlockInstance& blockInstance) {
        auto block = blockInstance.getBlock();
        auto& exblock = const_cast<Block&>(blockInstance.getBlockSource()->getExtraBlock(blockInstance.getPosition()));
        std::cout << "block: (" << getBlockId(block->getTypeName()) << ":" << block->getTileData() << ")\n"
                  << block->getNbt()->toPrettySNBT() << std::endl;
        player->sendText("block: (" + fto_string(getBlockId(block->getTypeName())) + ":" +
                         fto_string(block->getTileData()) + ")\n" + block->getNbt()->toPrettySNBT(true));
        if (&exblock != BedrockBlocks::mAir) {
            std::cout << "exBlock: (" << getBlockId(exblock.getTypeName()) << ":" << exblock.getTileData() << ")\n"
                      << exblock.getNbt()->toPrettySNBT() << std::endl;
            player->sendText("block: (" + fto_string(getBlockId(exblock.getTypeName())) + ":" +
                             fto_string(exblock.getTileData()) + ")\n" + exblock.getNbt()->toPrettySNBT(true));
        }
        if (blockInstance.hasBlockEntity()) {
            std::cout << "blockEntity:\n" << blockInstance.getBlockEntity()->getNbt()->toPrettySNBT() << std::endl;
            player->sendText("blockEntity:\n" + blockInstance.getBlockEntity()->getNbt()->toPrettySNBT(true));
        }
        return true;
    }

    ////////
    bool CyclerTool::leftClick(Player* player, BlockInstance& blockInstance) {
        // auto bs = Level::getBlockSource(player);
        return false;
    }
    bool CyclerTool::rightClick(Player* player, BlockInstance& blockInstance) {
        // auto bs = Level::getBlockSource(player);
        return false;
    }

    ////////
    bool FloodFillTool::rightClick(Player* player, BlockInstance& blockInstance) {
        // auto bs = Level::getBlockSource(player);
        return false;
    }

    ////////
    bool TreeTool::rightClick(Player* player, BlockInstance& blockInstance) {
        auto bs = Level::getBlockSource(player);
        auto pos = blockInstance.getPosition();
        setBlockSimple(bs, pos, VanillaBlocks::mGrass);
        pos.y += 1;
        setBlockSimple(bs, pos, const_cast<Block*>(VanillaBlocks::mSapling));
        auto& legacyBlock = blockInstance.getBlock()->getLegacyBlock();
        return ((Sapling&)legacyBlock).pgrowTree(*bs, pos, Global<Level>->getRandom(), false);
    }

    ////////
    bool DelTreeTool::rightClick(Player* player, BlockInstance& blockInstance) {
        return false;
    }

}  // namespace worldedit