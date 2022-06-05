//
// Created by OEOTYAN on 2021/2/8.
//

#include "Regions.h"
#include "MC/Level.hpp"
#include "MC/BlockSource.hpp"
#include "MC/Block.hpp"
#include "region/ChangeRegion.hpp"
#include <MC/BedrockBlocks.hpp>
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
    void Region::forTopBlockInRegion(
        const std::function<void(const BlockPos&)>& todo) {
        auto blockSource = Level::getBlockSource(dimensionID);
        for (int x = boundingBox.bpos1.x; x <= boundingBox.bpos2.x; x++)
            for (int z = boundingBox.bpos1.z; z <= boundingBox.bpos2.z; z++) {
                auto topy = blockSource->getHeightmap(x, z);
                if (topy < boundingBox.bpos2.y) {
                    for (int y = std::max(static_cast<int>(topy),
                                          boundingBox.bpos1.y + 1);
                         y <= boundingBox.bpos2.y; y++) {
                        if (&blockSource->getBlock({x, y, z}) ==
                                BedrockBlocks::mAir &&
                            &blockSource->getBlock({x, y - 1, z}) !=
                                BedrockBlocks::mAir &&
                            contains({x, y, z})) {
                            todo({x, y, z});
                            break;
                        }
                    }
                } else {
                    for (int y = boundingBox.bpos2.y;
                         y >= boundingBox.bpos1.y + 1; y--) {
                        if (&blockSource->getBlock({x, y, z}) ==
                                BedrockBlocks::mAir &&
                            &blockSource->getBlock({x, y - 1, z}) !=
                                BedrockBlocks::mAir &&
                            contains({x, y, z})) {
                            todo({x, y, z});
                            break;
                        }
                    }
                }
            }
    }

    std::vector<double> Region::getHeightMap() {
        int sizex = boundingBox.bpos2.x - boundingBox.bpos1.x + 1;
        int sizez = boundingBox.bpos2.z - boundingBox.bpos1.z + 1;
        std::vector<double> res(sizex * sizez, -1e200);

        auto blockSource = Level::getBlockSource(dimensionID);
        for (int x = boundingBox.bpos1.x; x <= boundingBox.bpos2.x; x++)
            for (int z = boundingBox.bpos1.z; z <= boundingBox.bpos2.z; z++) {
                auto topy = blockSource->getHeightmap(x, z)-1;
                if (topy <= boundingBox.bpos2.y &&
                    topy >= boundingBox.bpos1.y) {
                    int cx = x - boundingBox.bpos1.x;
                    int cz = z - boundingBox.bpos1.z;
                    res[cx * sizez + cz] = topy + 0.5;
                }
            }
        return res;
    }

    void Region::applyHeightMap(const std::vector<double>& data) {
        int sizex = boundingBox.bpos2.x - boundingBox.bpos1.x + 1;
        int sizez = boundingBox.bpos2.z - boundingBox.bpos1.z + 1;
        auto blockSource = Level::getBlockSource(dimensionID);
        for (int posx = boundingBox.bpos1.x; posx <= boundingBox.bpos2.x;
             posx++)
            for (int posz = boundingBox.bpos1.z; posz <= boundingBox.bpos2.z;
                 posz++) {
                BlockPos pos = {posx, blockSource->getHeightmap(posx, posz),
                                posz};
                pos.y -= 1;
                if (pos.y > boundingBox.bpos2.y ||
                    pos.y < boundingBox.bpos1.y) {
                    continue;
                }
                int x = pos.x - boundingBox.bpos1.x;
                int z = pos.z - boundingBox.bpos1.z;
                int index = x * sizez + z;

                if (data[index] < -1e150) {
                    return;
                }

                int curHeight = pos.y;
                int newHeight = std::min(boundingBox.bpos2.y,
                                         static_cast<int>(floor(data[index])));
                int xr = pos.x;
                int zr = pos.z;
                double scale =
                    static_cast<double>(curHeight - boundingBox.bpos1.y) /
                    static_cast<double>(newHeight - boundingBox.bpos1.y);

                if (newHeight > curHeight) {
                    auto* top = const_cast<Block*>(
                        &blockSource->getBlock({xr, curHeight, zr}));

                    setBlockSimple(blockSource, {xr, newHeight, zr}, top);

                    for (int y = newHeight - 1 - boundingBox.bpos1.y; y >= 0;
                         y--) {
                        int copyFrom = static_cast<int>(floor(y * scale));
                        setBlockSimple(
                            blockSource, {xr, boundingBox.bpos1.y + y, zr},
                            const_cast<Block*>(&blockSource->getBlock(
                                {xr, boundingBox.bpos1.y + copyFrom, zr})));
                    }
                } else if (curHeight > newHeight) {
                    for (int y = 0; y < newHeight - boundingBox.bpos1.y; y++) {
                        int copyFrom = static_cast<int>(floor(y * scale));
                        setBlockSimple(
                            blockSource, {xr, boundingBox.bpos1.y + y, zr},
                            const_cast<Block*>(&blockSource->getBlock(
                                {xr, boundingBox.bpos1.y + copyFrom, zr})));
                    }

                    setBlockSimple(blockSource, {xr, newHeight, zr},
                                   const_cast<Block*>(&blockSource->getBlock(
                                       {xr, curHeight, zr})));

                    for (int y = newHeight + 1; y <= curHeight; y++) {
                        setBlockSimple(blockSource, {xr, y, zr});
                    }
                }
            };
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
            case CYLINDER:
                 return new CylinderRegion(box, dim);
                //return new CuboidRegion(box, dim);
            default:
                return new CuboidRegion(box, dim);
        }
    }
}  // namespace worldedit