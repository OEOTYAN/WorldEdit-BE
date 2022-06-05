#include "BlockNBTSet.hpp"
#include "region/ChangeRegion.hpp"

namespace worldedit {

    double posfmod(double x, double y) { return x - floor(x / y) * y; }

    void BlockNBTSet::setBlock(const BlockPos& pos,
                               BlockSource* blockSource) const {
        setBlockSimple(blockSource, pos, block, exblock);
        if (hasBlockEntity) {
            auto blockInstance = blockSource->getBlockInstance(pos);
            if (blockInstance.hasBlockEntity()) {
                blockInstance.getBlockEntity()->setNbt(
                    CompoundTag::fromBinaryNBT(blockEntity).get());
            }
        }
    }
    void BlockNBTSet::setBlock(const BlockPos& pos,
                               BlockSource* blockSource,
                               Rotation rotation,
                               Mirror mirror) const {
        setBlockSimple(
            blockSource, pos,
            const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(
                *block,
                rotation, mirror)),
            const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(
                *exblock,
                rotation, mirror)));
        if (hasBlockEntity) {
            auto blockInstance = blockSource->getBlockInstance(pos);
            if (blockInstance.hasBlockEntity()) {
                blockInstance.getBlockEntity()->setNbt(
                    CompoundTag::fromBinaryNBT(blockEntity).get());
            }
        }
    }
}  // namespace worldedit
