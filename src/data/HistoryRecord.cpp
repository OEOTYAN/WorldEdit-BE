#include "HistoryRecord.h"
#include "data/LocalContext.h"

namespace we {
size_t HistoryRecord::getEstimatedSize() const noexcept {
    size_t totalSize  = sizeof(*this);
    totalSize        += newData.getEstimatedSize();
    totalSize        += oldData.getEstimatedSize();
    return totalSize;
}


size_t HistoryRecord::undo(LocalContext& context, BlockSource& region) const {
    size_t totalUndone = 0;
    for (auto it = oldData.operations.rbegin(); it != oldData.operations.rend(); ++it) {
        if (it->apply(context, region)) {
            // successfully applied
            totalUndone++;
        }
    }
    return totalUndone;
}
size_t HistoryRecord::apply(LocalContext& context, BlockSource& region) const {
    size_t totalApplied = 0;
    for (auto const& op : newData.operations) {
        if (op.apply(context, region)) {
            // successfully applied
            totalApplied++;
        }
    }
    return totalApplied;
}

void HistoryRecord::record(
    LocalContext&            context,
    BlockSource&             region,
    OperationData::Operation op
) {
    if (context.masked(region, op.pos)) {
        // position is masked, do not record
        return;
    }
    if (positionMap.contains(op.pos)) {
        // already recorded
        return;
    }
    positionMap[op.pos] = newData.operations.size();
    oldData.operations.push_back(op.record(context, region));
    newData.operations.push_back(std::move(op));
}

} // namespace we