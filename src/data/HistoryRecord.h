#pragma once

#include "data/OperationData.h"
#include "worldedit/Global.h"

namespace we {
class LocalContext;
class HistoryRecord {
public:
    size_t getEstimatedSize() const noexcept;

    size_t size() const noexcept { return positionMap.size(); }

    size_t undo(
        LocalContext&                              context,
        BlockSource&                               source,
        brstd::function_ref<bool(BlockPos const&)> canUndo
    ) const;
    size_t redo(
        LocalContext&                              context,
        BlockSource&                               source,
        brstd::function_ref<bool(BlockPos const&)> canRedo
    ) const;
    size_t apply(LocalContext& context, BlockSource& source) const;

    void record(LocalContext& context, BlockSource& source, OperationData::Operation op);

private:
    OperationData                  newData;
    OperationData                  oldData;
    ll::DenseMap<BlockPos, size_t> positionMap;
};

} // namespace we