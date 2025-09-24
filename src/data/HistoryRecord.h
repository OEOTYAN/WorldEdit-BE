#pragma once

#include "data/OperationData.h"
#include "worldedit/Global.h"

namespace we {
class LocalContext;
class HistoryRecord {
public:
    size_t getEstimatedSize() const noexcept;

    size_t undo(
        LocalContext&                              context,
        BlockSource&                               region,
        brstd::function_ref<bool(BlockPos const&)> canUndo
    ) const;
    size_t redo(
        LocalContext&                              context,
        BlockSource&                               region,
        brstd::function_ref<bool(BlockPos const&)> canRedo
    ) const;
    size_t apply(LocalContext& context, BlockSource& region) const;

    void record(LocalContext& context, BlockSource& region, OperationData::Operation op);

private:
    OperationData                  newData;
    OperationData                  oldData;
    ll::DenseMap<BlockPos, size_t> positionMap;
};

} // namespace we