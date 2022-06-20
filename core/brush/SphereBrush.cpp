//
// Created by OEOTYAN on 2022/06/10.
//

#include "SphereBrush.h"
#include "builder/SimpleBuilder.h"
namespace worldedit {
    SphereBrush::SphereBrush(unsigned short s, BlockPattern* bp, bool a) : Brush(s, bp), hollow(a) {}
    long long SphereBrush::set(Player* player, BlockInstance blockInstance) {
        return SimpleBuilder::buildSphere(blockInstance.getPosition(), blockInstance.getDimensionId(),
                                          player->getXuid(), pattern, size, hollow,mask);
    }
}  // namespace worldedit