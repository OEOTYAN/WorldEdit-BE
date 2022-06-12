//
// Created by OEOTYAN on 2022/06/10.
//
#include "BlockNBTSet.hpp"
#include "region/ChangeRegion.hpp"

namespace worldedit {

    BlockNBTSet::BlockNBTSet(BlockInstance& blockInstance) : hasBlock(true) {
        block   = blockInstance.getBlock();
        exblock = &const_cast<Block&>((blockInstance.getBlockSource()->getExtraBlock(blockInstance.getPosition())));
        if (blockInstance.hasBlockEntity()) {
            hasBlockEntity = true;
            blockEntity    = blockInstance.getBlockEntity()->getNbt()->toBinaryNBT();
        }
    }

    void BlockNBTSet::setBlock(const BlockPos& pos, BlockSource* blockSource) const {
        setBlockSimple(blockSource, pos, block, exblock);
        if (hasBlockEntity) {
            auto blockInstance = blockSource->getBlockInstance(pos);
            if (blockInstance.hasBlockEntity()) {
                blockInstance.getBlockEntity()->setNbt(CompoundTag::fromBinaryNBT(blockEntity).get());
            }
        }
    }
    void BlockNBTSet::setBlock(const BlockPos& pos, BlockSource* blockSource, Rotation rotation, Mirror mirror) const {
        setBlockSimple(blockSource, pos,
                       const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(*block, rotation, mirror)),
                       const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(*exblock, rotation, mirror)));
        if (hasBlockEntity) {
            auto blockInstance = blockSource->getBlockInstance(pos);
            if (blockInstance.hasBlockEntity()) {
                blockInstance.getBlockEntity()->setNbt(CompoundTag::fromBinaryNBT(blockEntity).get());
            }
        }
    }
}  // namespace worldedit
