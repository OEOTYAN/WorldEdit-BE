//
// Created by OEOTYAN on 2022/06/10.
//

#include "ImageHeightmapBrush.h"
#include "store/Patterns.h"
#include "mc/Dimension.hpp"
#include "WorldEdit.h"

namespace worldedit {
    ImageHeightmapBrush::ImageHeightmapBrush(unsigned short s, int a, Texture2D const& tex, bool r)
        : Brush(s, nullptr), height(a), texture(tex), rotation(r) {}

    long long ImageHeightmapBrush::set(Player* player, BlockInstance blockInstance) {
        if (blockInstance == BlockInstance::Null) {
            return -2;
        }
        auto pos = blockInstance.getPosition();

        auto xuid = player->getXuid();
        auto& playerData = getPlayersData(xuid);
        auto dimID = player->getDimensionId();
        auto blockSource = &player->getDimensionBlockSource();
        auto range =
            reinterpret_cast<Dimension*>(Global<Level>->getDimension(dimID).mHandle.lock().get())->getHeightRange();
        int minY = 2147483647;
        int maxY = -2147483648;
        int volume = (size * 2 + 1) * (size * 2 + 1);
        std::vector<double> map(volume, -1e200);

        for (int dx = -size; dx <= size; dx++)
            for (int dz = -size; dz <= size; dz++) {
                auto posk = pos + BlockPos(dx, 0, dz);
                posk.y = getHighestTerrainBlock(blockSource, posk.x, posk.z, range.min, range.max - 1, mask);
                if (range.min > posk.y) {
                    continue;
                }
                maxY = std::max(maxY, posk.y);
                minY = std::min(minY, posk.y);
                map[(dx + size) * (size * 2 + 1) + dz + size] = posk.y + 0.5;
            }
        if (height > 0) {
            maxY += height;
        } else {
            minY += height;
        }
        maxY = std::min(maxY, range.max - 1);
        minY = std::max(minY, (int)range.min);

        BoundingBox box({pos.x - size, minY, pos.z - size}, {pos.x + size, maxY, pos.z + size});

        if (playerData.maxHistoryLength > 0) {
            auto& history = playerData.getNextHistory();
            history = std::move(Clipboard(box.max - box.min));
            history.playerRelPos.x = dimID;
            history.playerPos = box.min;
            box.forEachBlockInBox([&](const BlockPos& pos) {
                auto localPos = pos - box.min;
                auto blockInstance = blockSource->getBlockInstance(pos);
                history.storeBlock(blockInstance, localPos);
            });
        }

        Sampler sampler(SamplerType::Bilinear, EdgeType::FLIP);

        long long iter = 0;

        std::function<void(double&, double&)> rotate;

        if (rotation) {
            auto angle = player->getRotation().y;
            if (angle >= -45 && angle < 45) {
                rotate = [&](double& u, double& v) {};
            } else if (angle >= 45 && angle < 135) {
                rotate = [&](double& u, double& v) {
                    std::swap(u, v);
                    v = 1 - v;
                };
            } else if (angle >= 135 || angle < -135) {
                rotate = [&](double& u, double& v) {
                    u = 1 - u;
                    v = 1 - v;
                };
            } else {
                rotate = [&](double& u, double& v) {
                    std::swap(u, v);
                    u = 1 - u;
                };
            }
        } else {
            rotate = [&](double& u, double& v) {};
        }

        EvalFunctions f;
        f.setbs(blockSource);
        f.setbox(box);
        phmap::flat_hash_map<std::string, double> variables;
        playerData.setVarByPlayer(variables);

        auto playerPos = player->getPosition();

        Vec3 center = pos.toVec3();

        for (int dx = -size; dx <= size; dx++)
            for (int dz = -size; dz <= size; dz++) {
                auto posk = pos + BlockPos(dx, 0, dz);
                double mapp = map[(dx + size) * (size * 2 + 1) + dz + size];
                int originY = (int)floor(mapp);
                double u = (dx + size) * 0.5 / size;
                double v = (dz + size) * 0.5 / size;
                rotate(u, v);
                double mapPoint = colorToHeight(texture.sample(sampler, u, v));
                posk.y = (int)floor(mapp + mapPoint * height);
                posk.y = std::max(posk.y, (int)range.min);
                posk.y = std::min(posk.y, (int)range.max - 1);
                if (posk.y > originY) {
                    auto* block = &blockSource->getBlock({posk.x, originY, posk.z});
                    for (int i = originY + 1; i <= posk.y; ++i) {
                        BlockPos iPos(posk.x, i, posk.z);
                        setFunction(variables, f, box, playerPos, iPos, center);
                        maskFunc(f, variables, [&]() mutable {
                            iter +=
                                playerData.setBlockSimple(blockSource, f, variables, iPos,block);
                        });
                    }
                } else if (posk.y < originY) {
                    for (int i = posk.y; i <= originY; ++i) {
                        BlockPos iPos(posk.x, i, posk.z);
                        setFunction(variables, f, box, playerPos, iPos, center);
                        maskFunc(f, variables,
                                 [&]() mutable { iter += playerData.setBlockSimple(blockSource, f, variables, iPos); });
                    }
                }
            }

        return iter;
    }
}  // namespace worldedit