//
// Created by OEOTYAN on 2022/06/10.
//

#include "CylinderBrush.h"
#include "builder/SimpleBuilder.h"
#include "store/Patterns.h"
namespace worldedit {
    CylinderBrush::CylinderBrush(unsigned short s, std::unique_ptr<Pattern> bp, int h, bool a)
        : Brush(s, std::move(bp)), height(h), hollow(a) {}
    long long CylinderBrush::set(Player* player, BlockInstance blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return -2;
        }
        return SimpleBuilder::buildCylinder(blockInstance.getPosition(), blockInstance.getDimensionId(),
                                            player->getXuid(), pattern.get(), size, height, hollow, mask);
    }
}  // namespace worldedit