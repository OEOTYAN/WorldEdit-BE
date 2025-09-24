#include "OperationData.h"
#include "data/LocalContext.h"

namespace we {
bool OperationData::BlockOperation::apply(
    LocalContext&   context,
    BlockSource&    region,
    BlockPos const& pos
) const {
    bool applied = false;
    if (block) {
        applied |= context.setBlock(region, pos, *block, blockActor);
    }
    if (extraBlock) {
        applied |= context.setExtraBlock(region, pos, *extraBlock);
    }
    if (biome) {
        applied |= context.setBiome(region, pos, *biome);
    }
    return applied;
}

OperationData::BlockOperation OperationData::BlockOperation::record(
    LocalContext&   context,
    BlockSource&    region,
    BlockPos const& pos
) const {
    OperationData::BlockOperation result;
    if (block) {
        result.block      = &region.getBlock(pos);
        result.extraBlock = &region.getExtraBlock(pos);
        if (optional_ref<BlockActor> oldBlockActor = region.getBlockEntity(pos);
            oldBlockActor) {
            CompoundTag nbt;
            if (oldBlockActor->save(nbt, SaveContext{})) {
                result.blockActor = BlockActor::create(nbt, pos);
            }
        }
    }
    if (biome) {
        result.biome = &region.getBiome(pos);
    }
    return result;
}
} // namespace we
