//
// Created by OEOTYAN on 2022/06/10.
//
#include "Global.h"
#include "MC/BlockPos.hpp"
#include <MC/Player.hpp>
#include "SimpleBuilder.h"
#include "WorldEdit.h"
#include <MC/Dimension.hpp>
#include "eval/Eval.h"
#include "store/BlockPattern.hpp"
#include "region/ChangeRegion.hpp"
namespace worldedit {

    long long SimpleBuilder::buildCylinder(BlockPos pos,
                                           int dim,
                                           std::string xuid,
                                           BlockPattern* blockPattern,
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
        auto range = Global<Level>->getDimension(dim)->getHeightRange();

        if (pos.y < range.min) {
            pos.y = range.min;
        } else if (pos.y + height - 1 > range.max) {
            height = range.max - pos.y + 1;
        }
        BoundingBox box({pos.x - radius, pos.y, pos.z - radius}, {pos.x + radius, pos.y + height, pos.z + radius});
        long long i = 0;

        auto& mod = worldedit::getMod();
        auto blockSource = Level::getBlockSource(dim);

        INNERIZE_GMASK

        std::function<void(EvalFunctions&, std::unordered_map<std::string, double> const&,
                           std::function<void()> const&)>
            maskLambda;
        if (mask != "") {
            maskLambda = [&](EvalFunctions& func, const std::unordered_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable {
                if (cpp_eval::eval<double>(mask, var, func) > 0.5) {
                    todo();
                }
            };
        } else {
            maskLambda = [&](EvalFunctions& func, const std::unordered_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable { todo(); };
        }

        auto* player = Global<Level>->getPlayer(xuid);

        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        EvalFunctions f;
        f.setbs(blockSource);
        f.setbox(box);
        std::unordered_map<std::string, double> variables;
        setFunction(variables, playerPos, playerRot);

        if (mod.maxHistoryLength > 0) {
            auto history = mod.getPlayerNextHistory(xuid);
            *history = Clipboard(box.max - box.min);
            history->playerRelPos.x = dim;
            history->playerPos = box.min;

            box.forEachBlockInBox([&](const BlockPos& blockPos) {
                auto localPos = blockPos - box.min;
                auto blockInstance = blockSource->getBlockInstance(blockPos);
                history->storeBlock(blockInstance, localPos);
            });
        }

        BlockPos blockPos;

        double R = radius;
        R += 0.5;
        int X, Y, p;
        X = 0;
        Y = (int)R;
        p = (int)(3 - 2 * R);
        for (; X <= Y; X++) {
            int k = hollow ? Y : X;
            for (int Yi = k; Yi <= Y; Yi++) {
                for (int y = 0; y < height; y++) {
                    blockPos = pos;
                    blockPos += {X, y, Yi};

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.z = pos.z - Yi;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.x = pos.x - X;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.z = pos.z + Yi;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.x = pos.x + Yi;
                    blockPos.z = pos.z + X;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.z = pos.z - X;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.x = pos.x - Yi;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.z = pos.z + X;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });
                }
            }
            if (p >= 0) {
                p += 4 * (X - Y) + 10;
                Y--;
            } else {
                p += 4 * X + 6;
            }
        }

        return i;
    }

    long long SimpleBuilder::buildSphere(BlockPos pos,
                                         int dim,
                                         std::string xuid,
                                         BlockPattern* blockPattern,
                                         unsigned short radius,
                                         bool hollow,
                                         std::string mask) {
        BoundingBox box(pos - radius, pos + radius);
        long long i = 0;

        auto& mod = worldedit::getMod();
        auto blockSource = Level::getBlockSource(dim);

        INNERIZE_GMASK

        std::function<void(EvalFunctions&, std::unordered_map<std::string, double> const&,
                           std::function<void()> const&)>
            maskLambda;
        if (mask != "") {
            maskLambda = [&](EvalFunctions& func, const std::unordered_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable {
                if (cpp_eval::eval<double>(mask, var, func) > 0.5) {
                    todo();
                }
            };
        } else {
            maskLambda = [&](EvalFunctions& func, const std::unordered_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable { todo(); };
        }

        auto* player = Global<Level>->getPlayer(xuid);

        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        EvalFunctions f;
        f.setbs(blockSource);
        f.setbox(box);
        std::unordered_map<std::string, double> variables;
        setFunction(variables, playerPos, playerRot);

        if (mod.maxHistoryLength > 0) {
            auto history = mod.getPlayerNextHistory(xuid);
            *history = Clipboard(box.max - box.min);
            history->playerRelPos.x = dim;
            history->playerPos = box.min;

            box.forEachBlockInBox([&](const BlockPos& blockPos) {
                auto localPos = blockPos - box.min;
                auto blockInstance = blockSource->getBlockInstance(blockPos);
                history->storeBlock(blockInstance, localPos);
            });
        }

        BlockPos blockPos;

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
                    blockPos = pos;
                    blockPos += {x, y, z};

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.y = pos.y - y;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.z = pos.z - z;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.x = pos.x - x;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.y = pos.y + y;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.x = pos.x + x;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.x = pos.x - x;
                    blockPos.z = pos.z + z;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });

                    blockPos.y = pos.y - y;

                    setFunction(variables, f, box, playerPos, blockPos, pos.toVec3() + 0.5f);

                    gMaskLambda(f, variables, [&]() mutable {
                        maskLambda(f, variables, [&]() mutable {
                            blockPattern->setBlock(variables, f, blockSource, blockPos);
                            ++i;
                        });
                    });
                }
            }
        }
        return i;
    }

    long long SimpleBuilder::buildCube(BlockPos pos,
                                       int dim,
                                       std::string xuid,
                                       BlockPattern* blockPattern,
                                       unsigned short size,
                                       bool hollow,
                                       std::string mask) {
        BoundingBox box(pos - size, pos + size);
        long long i = 0;

        auto& mod = worldedit::getMod();
        auto blockSource = Level::getBlockSource(dim);

        INNERIZE_GMASK

        std::function<void(EvalFunctions&, std::unordered_map<std::string, double> const&,
                           std::function<void()> const&)>
            maskLambda;
        if (mask != "") {
            maskLambda = [&](EvalFunctions& func, const std::unordered_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable {
                if (cpp_eval::eval<double>(mask, var, func) > 0.5) {
                    todo();
                }
            };
        } else {
            maskLambda = [&](EvalFunctions& func, const std::unordered_map<std::string, double>& var,
                             std::function<void()> const& todo) mutable { todo(); };
        }

        auto* player = Global<Level>->getPlayer(xuid);

        auto playerPos = player->getPosition();
        auto playerRot = player->getRotation();
        EvalFunctions f;
        f.setbs(blockSource);
        f.setbox(box);
        std::unordered_map<std::string, double> variables;
        setFunction(variables, playerPos, playerRot);

        if (mod.maxHistoryLength > 0) {
            auto history = mod.getPlayerNextHistory(xuid);
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

                gMaskLambda(f, variables, [&]() mutable {
                    maskLambda(f, variables, [&]() mutable {
                        blockPattern->setBlock(variables, f, blockSource, blockPos);
                        ++i;
                    });
                });
            }
        });
        return i;
    }
}  // namespace worldedit