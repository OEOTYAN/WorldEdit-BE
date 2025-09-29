#include "Move.h"
#include <algorithm>
#include <cmath>

namespace we::bot {

Move::Move(
    int                       x,
    int                       y,
    int                       z,
    int                       remainingBlocks,
    double                    cost,
    std::vector<BlockToBreak> toBreak,
    std::vector<BlockToPlace> toPlace,
    bool                      parkour
)
: BlockPos(x, y, z),
  remainingBlocks(remainingBlocks),
  cost(cost),
  toBreak(std::move(toBreak)),
  toPlace(std::move(toPlace)),
  parkour(parkour) {
}

Move::Move(
    BlockPos const&           pos,
    int                       remainingBlocks,
    double                    cost,
    std::vector<BlockToBreak> toBreak,
    std::vector<BlockToPlace> toPlace,
    bool                      parkour
)
: BlockPos(pos),
  remainingBlocks(remainingBlocks),
  cost(cost),
  toBreak(std::move(toBreak)),
  toPlace(std::move(toPlace)),
  parkour(parkour) {}

double Move::distanceSquared(Move const& other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    double dz = z - other.z;
    return dx * dx + dy * dy + dz * dz;
}

double Move::distance(Move const& other) const {
    return std::sqrt(distanceSquared(other));
}

Move Move::min(Move const& other) const {
    return Move(std::min(x, other.x), std::min(y, other.y), std::min(z, other.z), 0, 0);
}

Move Move::max(Move const& other) const {
    return Move(std::max(x, other.x), std::max(y, other.y), std::max(z, other.z), 0, 0);
}

bool Move::operator==(Move const& other) const {
    return x == other.x && y == other.y && z == other.z;
}

} // namespace we::bot