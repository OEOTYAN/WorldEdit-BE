#include "Math.h"

namespace we {
void plotLine(
    BlockPos const&                             pos0,
    BlockPos const&                             pos1,
    std::function<void(BlockPos const&)> const& todo
) {
    int x0 = pos0.x;
    int y0 = pos0.y;
    int z0 = pos0.z;
    int x1 = pos1.x;
    int y1 = pos1.y;
    int z1 = pos1.z;
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int dz = abs(z1 - z0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int sz = z0 < z1 ? 1 : -1;

    int dMax = std::max(dx, std::max(dy, dz));
    if (dMax == dx) {
        for (int domstep = 0; domstep <= dx; domstep++) {
            int tipx = x0 + domstep * sx;
            int tipy = (int)round(y0 + domstep / ((double)dx) * ((double)dy) * sy);
            int tipz = (int)round(z0 + domstep / ((double)dx) * ((double)dz) * sz);
            todo(BlockPos(tipx, tipy, tipz));
        }
    } else if (dMax == dy) {
        for (int domstep = 0; domstep <= dy; domstep++) {
            int tipx = (int)round(x0 + domstep / ((double)dy) * ((double)dx) * sx);
            int tipy = y0 + domstep * sy;
            int tipz = (int)round(z0 + domstep / ((double)dy) * ((double)dz) * sz);
            todo(BlockPos(tipx, tipy, tipz));
        }
    } else {
        for (int domstep = 0; domstep <= dz; domstep++) {
            int tipx = (int)round(x0 + domstep / ((double)dz) * ((double)dx) * sx);
            int tipy = (int)round(y0 + domstep / ((double)dz) * ((double)dy) * sy);
            int tipz = z0 + domstep * sz;
            todo(BlockPos(tipx, tipy, tipz));
        }
    }
    return;
}
} // namespace we
