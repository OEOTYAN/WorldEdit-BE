//
// Created by OEOTYAN on 2022/06/10.
//
#include "Globals.h"
#include "mc/BlockPos.hpp"
#include <mc/Player.hpp>
#include "SimpleBuilder.h"
#include "WorldEdit.h"
#include <mc/Dimension.hpp>
#include "eval/Eval.h"
#include "store/Patterns.h"
namespace worldedit {

    long long SimpleBuilder::buildCylinder(BlockPos pos,
                                           int dim,
                                           std::string xuid,
                                           Pattern* pattern,
                                           unsigned short radius,
                                           int height,
                                           bool hollow,
                                           std::string mask) {
        if (height == 0) {
            return -1;
        } else if (height < 0) {
            pos.y += height;
            height = -height;
        }
        auto range =
            reinterpret_cast<Dimension*>(Global<Level>->getDimension(dim).mHandle.lock().get())->getHeightRange();

        if (pos.y < range.min) {
            pos.y = range.min;
        } else if (pos.y + height - 1 > range.max) {
            height = range.max - pos.y + 1;
        }
        BoundingBox box({pos.x - radius, pos.y, pos.z - radius}, {pos.x + radius, pos.y + height, pos.z + radius});
        long long i = 0;

        auto& playerData = getPlayersData(xuid);
        auto blockSource = Level::getBlockSource(dim);

        std::function<void(EvalFunctions&, phmap::flat_hash_map<std::string, double> const&,
                           std::function<void()> const&)>
            maskLambda;
        if (mask != "") {
            maskLambda = [&](EvalFunctions& func, const phmap::flat_hash_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable {
                if (cpp_eval::eval<double>(mask, var, func) > 0.5) {
                    todo();
                }
            };
        } else {
            maskLambda = [&](EvalFunctions& func, const phmap::flat_hash_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable { todo(); };
        }

        auto* player = Global<Level>->getPlayer(xuid);

        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        EvalFunctions f;
        f.setbs(blockSource);
        f.setbox(box);
        phmap::flat_hash_map<std::string, double> variables;
        playerData.setVarByPlayer(variables);

        if (playerData.maxHistoryLength > 0) {
            auto history = playerData.getNextHistory();
            *history = Clipboard(box.max - box.min);
            history->playerRelPos.x = dim;
            history->playerPos = box.min;

            box.forEachBlockInBox([&](const BlockPos& blockPos) {
                auto localPos = blockPos - box.min;
                auto blockInstance = blockSource->getBlockInstance(blockPos);
                history->storeBlock(blockInstance, localPos);
            });
        }

        double R = radius;
        R += 0.5;
        int X, Y, p;
        X = 0;
        Y = (int)R;
        p = (int)(3 - 2 * R);

        phmap::flat_hash_set<BlockPos> bset;

        for (; X <= Y; X++) {
            int k = hollow ? Y : X;
            for (int Yi = k; Yi <= Y; Yi++) {
                BlockPos blockPos = pos;
                blockPos += {X, 0, Yi};
                bset.insert(blockPos);
                blockPos.z = pos.z - Yi;
                bset.insert(blockPos);
                blockPos.x = pos.x - X;
                bset.insert(blockPos);
                blockPos.z = pos.z + Yi;
                bset.insert(blockPos);
                blockPos.x = pos.x + Yi;
                blockPos.z = pos.z + X;
                bset.insert(blockPos);
                blockPos.z = pos.z - X;
                bset.insert(blockPos);
                blockPos.x = pos.x - Yi;
                bset.insert(blockPos);
                blockPos.z = pos.z + X;
                bset.insert(blockPos);
            }
            if (p >= 0) {
                p += 4 * (X - Y) + 10;
                Y--;
            } else {
                p += 4 * X + 6;
            }
        }
        for (auto b : bset)
            for (int y = 0; y < height; y++) {
                setFunction(variables, f, box, playerPos, b, pos.toVec3() + 0.5f);
                maskLambda(f, variables, [&]() mutable { i += pattern->setBlock(variables, f, blockSource, b); });
                b.y += 1;
            }

        return i;
    }

    long long SimpleBuilder::buildSphere(BlockPos pos,
                                         int dim,
                                         std::string xuid,
                                         Pattern* pattern,
                                         unsigned short radius,
                                         bool hollow,
                                         std::string mask) {
        BoundingBox box(pos - radius, pos + radius);
        long long i = 0;

        auto& playerData = getPlayersData(xuid);
        auto blockSource = Level::getBlockSource(dim);

        std::function<void(EvalFunctions&, phmap::flat_hash_map<std::string, double> const&,
                           std::function<void()> const&)>
            maskLambda;
        if (mask != "") {
            maskLambda = [&](EvalFunctions& func, const phmap::flat_hash_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable {
                if (cpp_eval::eval<double>(mask, var, func) > 0.5) {
                    todo();
                }
            };
        } else {
            maskLambda = [&](EvalFunctions& func, const phmap::flat_hash_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable { todo(); };
        }

        auto* player = Global<Level>->getPlayer(xuid);

        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        EvalFunctions f;
        f.setbs(blockSource);
        f.setbox(box);
        phmap::flat_hash_map<std::string, double> variables;
        playerData.setVarByPlayer(variables);

        if (playerData.maxHistoryLength > 0) {
            auto history = playerData.getNextHistory();
            *history = Clipboard(box.max - box.min);
            history->playerRelPos.x = dim;
            history->playerPos = box.min;

            box.forEachBlockInBox([&](const BlockPos& blockPos) {
                auto localPos = blockPos - box.min;
                auto blockInstance = blockSource->getBlockInstance(blockPos);
                history->storeBlock(blockInstance, localPos);
            });
        }

        phmap::flat_hash_set<BlockPos> bset;
        double R = radius + 0.5;
        double nextXn = 0;
        for (int x = 0; x <= radius; ++x) {
            double xn = nextXn;
            nextXn = (x + 1) / R;
            double nextYn = 0;
            for (int y = 0; y <= radius; ++y) {
                double yn = nextYn;
                nextYn = (y + 1) / R;
                double nextZn = 0;
                for (int z = 0; z <= radius; ++z) {
                    double zn = nextZn;
                    nextZn = (z + 1) / R;
                    if (xn * xn + yn * yn + zn * zn > 1) {
                        continue;
                    }
                    if (hollow) {
                        if (nextXn * nextXn + yn * yn + zn * zn <= 1 && xn * xn + nextYn * nextYn + zn * zn <= 1 &&
                            xn * xn + yn * yn + nextZn * nextZn <= 1) {
                            continue;
                        }
                    }

                    BlockPos blockPos = pos;
                    blockPos += {x, y, z};
                    bset.insert(blockPos);
                    blockPos.y = pos.y - y;
                    bset.insert(blockPos);
                    blockPos.z = pos.z - z;
                    bset.insert(blockPos);
                    blockPos.x = pos.x - x;
                    bset.insert(blockPos);
                    blockPos.y = pos.y + y;
                    bset.insert(blockPos);
                    blockPos.x = pos.x + x;
                    bset.insert(blockPos);
                    blockPos.x = pos.x - x;
                    blockPos.z = pos.z + z;
                    bset.insert(blockPos);
                    blockPos.y = pos.y - y;
                    bset.insert(blockPos);
                }
            }
        }
        for (auto& b : bset) {
            setFunction(variables, f, box, playerPos, b, pos.toVec3() + 0.5f);
            maskLambda(f, variables, [&]() mutable { i += pattern->setBlock(variables, f, blockSource, b); });
        }
        return i;
    }

    long long SimpleBuilder::buildCube(BlockPos pos,
                                       int dim,
                                       std::string xuid,
                                       Pattern* pattern,
                                       unsigned short size,
                                       bool hollow,
                                       std::string mask) {
        BoundingBox box(pos - size, pos + size);
        long long i = 0;

        auto& playerData = getPlayersData(xuid);
        auto blockSource = Level::getBlockSource(dim);

        std::function<void(EvalFunctions&, phmap::flat_hash_map<std::string, double> const&,
                           std::function<void()> const&)>
            maskLambda;
        if (mask != "") {
            maskLambda = [&](EvalFunctions& func, const phmap::flat_hash_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable {
                if (cpp_eval::eval<double>(mask, var, func) > 0.5) {
                    todo();
                }
            };
        } else {
            maskLambda = [&](EvalFunctions& func, const phmap::flat_hash_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable { todo(); };
        }

        auto* player = Global<Level>->getPlayer(xuid);

        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        EvalFunctions f;
        f.setbs(blockSource);
        f.setbox(box);
        phmap::flat_hash_map<std::string, double> variables;
        playerData.setVarByPlayer(variables);

        if (playerData.maxHistoryLength > 0) {
            auto history = playerData.getNextHistory();
            *history = Clipboard(box.max - box.min);
            history->playerRelPos.x = dim;
            history->playerPos = box.min;

            box.forEachBlockInBox([&](const BlockPos& blockPos) {
                auto localPos = blockPos - box.min;
                auto blockInstance = blockSource->getBlockInstance(blockPos);
                history->storeBlock(blockInstance, localPos);
            });
        }

        box.forEachBlockInBox([&](const BlockPos blockPos) {
            if (!hollow || (blockPos.x == box.min.x || blockPos.x == box.max.x || blockPos.y == box.min.y ||
                            blockPos.y == box.max.y || blockPos.z == box.min.z || blockPos.z == box.max.z)) {
                setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                maskLambda(f, variables, [&]() mutable {
                    pattern->setBlock(variables, f, blockSource, blockPos);
                    ++i;
                });
            }
        });
        return i;
    }
}  // namespace worldedit