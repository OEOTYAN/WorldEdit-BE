#include "BlockNBTSet.hpp"
#include "region/changeRegion.hpp"

namespace worldedit {
    void BlockNBTSet::setBlock(const BlockPos& pos, int dimID) const {
        auto blockSource = Level::getBlockSource(dimID);
        setBlockSimple(
            blockSource, pos,
            Block::create(CompoundTag::fromBinaryNBT(block).get()),
            Block::create(CompoundTag::fromBinaryNBT(exblock).get()));
        if (hasBlockEntity) {
            auto blockInstance = blockSource->getBlockInstance(pos);
            if (blockInstance.hasBlockEntity()) {
                blockInstance.getBlockEntity()->setNbt(
                    CompoundTag::fromBinaryNBT(blockEntity).get());
            }
        }
    }
    void BlockNBTSet::setBlock(const BlockPos& pos,
                               int dimID,
                               Rotation rotation,
                               Mirror mirror) const {
        auto blockSource = Level::getBlockSource(dimID);

        setBlockSimple(
            blockSource, pos,
            const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(
                *Block::create(CompoundTag::fromBinaryNBT(block).get()),
                rotation, mirror)),
            const_cast<Block*>(VanillaBlockStateTransformUtils::transformBlock(
                *Block::create(CompoundTag::fromBinaryNBT(exblock).get()),
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
