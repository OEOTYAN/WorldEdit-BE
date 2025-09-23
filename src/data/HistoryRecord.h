#pragma once

#include "worldedit/Global.h"
#include "data/OperationData.h"

namespace we {
class LocalContext;
class HistoryRecord {
public:
     size_t getEstimatedSize() const noexcept;

     size_t undo(LocalContext& context, BlockSource& region) const;
     size_t apply(LocalContext& context, BlockSource& region) const;

     void record(LocalContext& context, BlockSource& region, OperationData::Operation op);

 private:
     OperationData newData;
     OperationData oldData;
};

} // namespace we