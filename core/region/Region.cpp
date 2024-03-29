//
// Created by OEOTYAN on 2021/2/8.
//

#include "Regions.h"
#include "mc/Level.hpp"
#include "mc/BlockSource.hpp"
#include "mc/Block.hpp"
#include <mc/BedrockBlocks.hpp>
#include <mc/Player.hpp>
#include "eval/Eval.h"
#include "WorldEdit.h"
namespace worldedit {
    void Region::forEachBlockInRegion(const std::function<void(const BlockPos&)>& todo) {
        for (int y = boundingBox.min.y; y <= boundingBox.max.y; ++y)
            for (int x = boundingBox.min.x; x <= boundingBox.max.x; ++x)
                for (int z = boundingBox.min.z; z <= boundingBox.max.z; ++z) {
                    if (contains({x, y, z})) {
                        todo({x, y, z});
                    }
                }
    }
    void Region::forEachBlockUVInRegion(const std::function<void(const BlockPos&, double, double)>& todo) {
        auto size = boundingBox.max - boundingBox.min + 1;

        if (size.x == 1) {
            for (int y = boundingBox.min.y; y <= boundingBox.max.y; ++y)
                for (int z = boundingBox.min.z; z <= boundingBox.max.z; ++z) {
                    BlockPos pos(boundingBox.max.x, y, z);
                    if (contains(pos)) {
                        auto localPos = pos - boundingBox.min;
                        todo(pos, (localPos.z + 0.5) / size.z, (localPos.y + 0.5) / size.y);
                    }
                }
        } else if (size.z == 1) {
            for (int y = boundingBox.min.y; y <= boundingBox.max.y; ++y)
                for (int x = boundingBox.min.x; x <= boundingBox.max.x; ++x) {
                    BlockPos pos(x, y, boundingBox.max.z);
                    if (contains(pos)) {
                        auto localPos = pos - boundingBox.min;
                        todo(pos, (localPos.x + 0.5) / size.x, (localPos.y + 0.5) / size.y);
                    }
                }
        } else {
            for (int z = boundingBox.min.z; z <= boundingBox.max.z; ++z)
                for (int x = boundingBox.min.x; x <= boundingBox.max.x; ++x) {
                    BlockPos pos(x, boundingBox.max.y, z);
                    if (contains(pos)) {
                        auto localPos = pos - boundingBox.min;
                        todo(pos, (localPos.x + 0.5) / size.x, (localPos.z + 0.5) / size.z);
                    }
                }
        }
    }
    void Region::forTopBlockInRegion(const std::function<void(const BlockPos&)>& todo) {
        auto blockSource = Level::getBlockSource(dimensionID);
        for (int x = boundingBox.min.x; x <= boundingBox.max.x; ++x)
            for (int z = boundingBox.min.z; z <= boundingBox.max.z; ++z) {
                auto topy = blockSource->getHeightmap(x, z);
                if (topy < boundingBox.max.y) {
                    for (int y = std::max(static_cast<int>(topy), boundingBox.min.y + 1); y <= boundingBox.max.y; ++y) {
                        if (&blockSource->getBlock({x, y, z}) == BedrockBlocks::mAir &&
                            &blockSource->getBlock({x, y - 1, z}) != BedrockBlocks::mAir && contains({x, y, z})) {
                            todo({x, y, z});
                            break;
                        }
                    }
                } else {
                    for (int y = boundingBox.max.y; y >= boundingBox.min.y + 1; y--) {
                        if (&blockSource->getBlock({x, y, z}) == BedrockBlocks::mAir &&
                            &blockSource->getBlock({x, y - 1, z}) != BedrockBlocks::mAir && contains({x, y, z})) {
                            todo({x, y, z});
                            break;
                        }
                    }
                }
            }
    }

    int Region::getHeighest(int x, int z) {
        for (int y = boundingBox.max.y; y >= boundingBox.min.y; y--) {
            if (contains({x, y, z})) {
                return y;
            }
        }
        return boundingBox.min.y - 1;
    }

    std::vector<double> Region::getHeightMap(std::string mask) {
        int sizex = boundingBox.max.x - boundingBox.min.x + 1;
        int sizez = boundingBox.max.z - boundingBox.min.z + 1;
        std::vector<double> res(sizex * sizez, -1e200);

        auto blockSource = Level::getBlockSource(dimensionID);
        for (int x = boundingBox.min.x; x <= boundingBox.max.x; ++x)
            for (int z = boundingBox.min.z; z <= boundingBox.max.z; ++z) {
                auto topy = getHighestTerrainBlock(blockSource, x, z, boundingBox.min.y, getHeighest(x, z), mask);
                if (topy >= boundingBox.min.y) {
                    int cx = x - boundingBox.min.x;
                    int cz = z - boundingBox.min.z;
                    res[cx * sizez + cz] = topy + 0.5;
                }
            }
        return res;
    }

    void Region::applyHeightMap(const std::vector<double>& data, std::string xuid, std::string mask) {
        auto& playerData = getPlayersData(xuid);
        auto* player = Global<Level>->getPlayer(xuid);
        int sizex = boundingBox.max.x - boundingBox.min.x + 1;
        int sizez = boundingBox.max.z - boundingBox.min.z + 1;
        auto blockSource = Level::getBlockSource(dimensionID);
        EvalFunctions f;
        f.setbs(blockSource);
        f.setbox(boundingBox);
        phmap::flat_hash_map<std::string, double> variables;
        playerData.setVarByPlayer(variables);
        auto playerPos = player->getPosition();
        Vec3 center = boundingBox.getCenter().toVec3();
        for (int posx = boundingBox.min.x; posx <= boundingBox.max.x; posx++)
            for (int posz = boundingBox.min.z; posz <= boundingBox.max.z; posz++) {
                auto topy = getHighestTerrainBlock(blockSource, posx, posz, boundingBox.min.y,
                                                   getHeighest(posx, posz) + 1, mask);
                BlockPos pos = {posx, topy, posz};
                if (!contains(pos)) {
                    continue;
                }
                int x = pos.x - boundingBox.min.x;
                int z = pos.z - boundingBox.min.z;
                int index = x * sizez + z;

                if (data[index] < -1e150) {
                    return;
                }

                int curHeight = pos.y;
                int newHeight = std::min(boundingBox.max.y, static_cast<int>(floor(data[index])));
                int xr = pos.x;
                int zr = pos.z;
                double scale = static_cast<double>(curHeight - boundingBox.min.y) /
                               static_cast<double>(newHeight - boundingBox.min.y);

                if (newHeight > curHeight) {
                    auto* top = &blockSource->getBlock({xr, curHeight, zr});

                    pos = {xr, newHeight, zr};

                    setFunction(variables, f, boundingBox, playerPos, pos, center);
                    playerData.setBlockSimple(blockSource, f, variables, pos, top);

                    for (int y = newHeight - 1 - boundingBox.min.y; y >= 0; y--) {
                        int copyFrom = static_cast<int>(floor(y * scale));
                        pos = {xr, boundingBox.min.y + y, zr};
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        playerData.setBlockSimple(
                            blockSource, f, variables, pos,
                           &blockSource->getBlock({xr, boundingBox.min.y + copyFrom, zr}));
                    }
                } else if (curHeight > newHeight) {
                    for (int y = 0; y < newHeight - boundingBox.min.y; ++y) {
                        int copyFrom = static_cast<int>(floor(y * scale));
                        pos = {xr, boundingBox.min.y + y, zr};
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        playerData.setBlockSimple(
                            blockSource, f, variables, pos,
                            &blockSource->getBlock({xr, boundingBox.min.y + copyFrom, zr}));
                    }

                    pos = {xr, newHeight, zr};
                    setFunction(variables, f, boundingBox, playerPos, pos, center);
                    playerData.setBlockSimple(blockSource, f, variables, pos,
                                             &blockSource->getBlock({xr, curHeight, zr}));

                    for (int y = newHeight + 1; y <= curHeight; ++y) {
                        pos = {xr, y, zr};
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        playerData.setBlockSimple(blockSource, f, variables, pos);
                    }
                }
            };
    }

    void Region::renderRegion() {
        if (selecting && dimensionID >= 0 && rendertick <= 0) {
            globalPT().drawCuboid(getBoundBox(), dimensionID, mce::ColorPalette::YELLOW);
            rendertick = 40;
        }
        rendertick--;
    };

    Region::Region(const BoundingBox& b, int dim) : boundingBox(b), dimensionID(dim) {}

    std::unique_ptr<Region> Region::createRegion(RegionType type, const BoundingBox& box, int dim) {
        switch (type) {
            case CUBOID:
                return std::make_unique<CuboidRegion>(CuboidRegion(box, dim));
            case EXPAND:
                return std::make_unique<ExpandRegion>(ExpandRegion(box, dim));
            case SPHERE:
                return std::make_unique<SphereRegion>(SphereRegion(box, dim));
            case POLY:
                return std::make_unique<PolyRegion>(PolyRegion(box, dim));
            case CONVEX:
                return std::make_unique<ConvexRegion>(ConvexRegion(box, dim));
            case CYLINDER:
                return std::make_unique<CylinderRegion>(CylinderRegion(box, dim));
            case LOFT:
                return std::make_unique<LoftRegion>(LoftRegion(box, dim));
            default:
                return std::make_unique<CuboidRegion>(CuboidRegion(box, dim));
        }
    }
}  // namespace worldedit