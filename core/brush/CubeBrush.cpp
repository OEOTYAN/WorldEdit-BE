//
// Created by OEOTYAN on 2022/06/11.
//

#include "CubeBrush.h"
#include "builder/SimpleBuilder.h"

namespace worldedit {
    CubeBrush::CubeBrush(unsigned short s, BlockPattern* bp, bool a) : Brush(s, bp), hollow(a) {}
    long long CubeBrush::set(Player* player, BlockInstance blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return -2;
        }
        return SimpleBuilder::buildCube(blockInstance.getPosition(), blockInstance.getDimensionId(), player->getXuid(),
                                        pattern, size, hollow,mask);
    }
}  // namespace worldedit