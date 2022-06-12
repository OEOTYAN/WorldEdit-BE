//
// Created by OEOTYAN on 2022/06/10.
//
#include "tool.h"
#include "region/ChangeRegion.hpp"
#include "string/StringTool.h"
#include "WorldEdit.h"
namespace worldedit {

    bool Tool::leftClick(Player* player, BlockInstance blockInstance) {
        return false;
    }
    bool Tool::rightClick(Player* player, BlockInstance blockInstance) {
        return false;
    }
    bool FarWand::leftClick(Player* player, BlockInstance blockInstance) {
        changeMainPos(player, blockInstance);
        return true;
    }
    bool FarWand::rightClick(Player* player, BlockInstance blockInstance) {
        changeVicePos(player, blockInstance);
        return true;
    }
    bool InfoTool::rightClick(Player* player, BlockInstance blockInstance) {
        auto  block   = blockInstance.getBlock();
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
}  // namespace worldedit