//
// Created by OEOTYAN on 2021/2/8.
//

#include "Regions.h"
namespace worldedit {
    void Region::forEachBlockInRegion(
        const std::function<void(const BlockPos&)>& todo) {
        for (int y = boundingBox.bpos1.y; y <= boundingBox.bpos2.y; y++)
            for (int x = boundingBox.bpos1.x; x <= boundingBox.bpos2.x; x++)
                for (int z = boundingBox.bpos1.z; z <= boundingBox.bpos2.z;
                     z++) {
                    if (contains({x, y, z})) {
                        todo({x, y, z});
                    }
                }
    }

    void Region::renderRegion() {
        if (selecting && dimensionID >= 0 && rendertick <= 0) {
            worldedit::spawnCuboidParticle(toRealAABB(this->getBoundBox()),
                                           GRAPHIC_COLOR::YELLOW, dimensionID);
            rendertick = 40;
        }
        rendertick--;
    };

    Region::Region(const BoundingBox& b, int dim)
        : boundingBox(b), dimensionID(dim) {}

    Region* Region::createRegion(RegionType type,
                                 const BoundingBox& box,
                                 int dim) {
        switch (type) {
            case CUBOID:
                return new CuboidRegion(box, dim);
            case EXPAND:
                return new ExpandRegion(box, dim);
            case SPHERE:
                return new SphereRegion(box, dim);
            case POLY:
                return new PolyRegion(box, dim);
            case CONVEX:
                return new ConvexRegion(box, dim);
            default:
                return new CuboidRegion(box, dim);
        }
    }
}  // namespace worldedit