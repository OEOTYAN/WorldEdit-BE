//
// Created by OEOTYAN on 2022/06/10.
//

#include "CylinderBrush.h"
#include "builder/SimpleBuilder.h"
namespace worldedit {
    CylinderBrush::CylinderBrush(unsigned short s, BlockPattern* bp, int h, bool a)
        : Brush(s, bp), height(h), hollow(a) {}
    long long CylinderBrush::set(Player* player, BlockInstance blockInstance) {
        return SimpleBuilder::buildCylinder(blockInstance.getPosition(), blockInstance.getDimensionId(),
                                            player->getXuid(), pattern, size, height, hollow,mask);
    }
}  // namespace worldedit