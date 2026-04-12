#pragma once

#include "worldedit/Global.h"
#include "Operation.h"

namespace we {
class LocalContext;
class Operations {
public:
    std::vector<Operation> operations;


    size_t getEstimatedSize() const noexcept {
        size_t totalSize  = sizeof(*this);
        totalSize        += operations.size() * sizeof(Operation);
        return totalSize;
    }
};

} // namespace we