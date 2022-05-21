#pragma once
//
// Created by OEOTYAN on 2022/5/15.
//
#ifndef WORLDEDIT_EVAL_H
#define WORLDEDIT_EVAL_H

#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockLegacy.hpp>
#include <MC/BlockSource.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/Dimension.hpp>
#include <MC/ItemStack.hpp>
#include "string/StringTool.h"
#include <LLAPI.h>
#include <ServerAPI.h>
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include <DynamicCommandAPI.h>
#include "particle/Graphics.h"
#include "WorldEdit.h"
#include "CppEval.h"
#include "pcg_random.hpp"
#include "FastNoise/FastNoise.h"

namespace worldedit {
    pcg32 rng(pcg_extras::seed_seq_from<std::random_device>{});
    class EvalFunctions {
        BlockPos here;
        std::uniform_real_distribution<double> uniform_dist{0.0, 1.0};
        BlockSource* blockSource;
        bool blockdataInitialized = false;

       public:
        void setPos(const BlockPos& pos) { here = pos; }
        void setbs(BlockSource* bs) {
            blockSource = bs;
            blockdataInitialized = true;
        }
        double operator()(const char* name, const std::vector<double>& params) {
            switch (do_hash(name)) {
                case do_hash("rand"):
                    if (params.size() == 0) {
                        return uniform_dist(rng);
                    } else if (params.size() == 2) {
                        return uniform_dist(rng) * (params[1] - params[0]) +
                               params[0];
                    }
                    break;
                case do_hash("sin"):
                    if (params.size() == 1)
                        return sin(params[0]);
                    break;
                case do_hash("abs"):
                    if (params.size() == 1)
                        return abs(params[0]);
                    break;
                case do_hash("cos"):
                    if (params.size() == 1)
                        return cos(params[0]);
                    break;
                case do_hash("sign"):
                    if (params.size() == 1)
                        return params[0] == 0.0
                                   ? 0.0
                                   : (params[0] > 0.0 ? 1.0 : -1.0);
                    break;
                case do_hash("lg"):
                    if (params.size() == 1)
                        return log10(params[0]);
                    break;
                case do_hash("ln"):
                    if (params.size() == 1)
                        return log(params[0]);
                    break;
                case do_hash("log2"):
                    if (params.size() == 1)
                        return log2(params[0]);
                    break;
                case do_hash("round"):
                    if (params.size() == 1)
                        return round(params[0]);
                    break;
                case do_hash("floor"):
                    if (params.size() == 1)
                        return floor(params[0]);
                    break;
                case do_hash("ceil"):
                    if (params.size() == 1)
                        return ceil(params[0]);
                    break;
                case do_hash("exp"):
                    if (params.size() == 1)
                        return exp(params[0]);
                    break;
                case do_hash("exp2"):
                    if (params.size() == 1)
                        return exp2(params[0]);
                    break;
                case do_hash("sqrt"):
                    if (params.size() == 1)
                        return sqrt(params[0]);
                    break;
                case do_hash("tan"):
                    if (params.size() == 1)
                        return tan(params[0]);
                    break;
                case do_hash("atan"):
                    if (params.size() == 1)
                        return atan(params[0]);
                    break;
                case do_hash("atan2"):
                    if (params.size() == 2)
                        return atan2(params[0], params[1]);
                    break;
                case do_hash("asin"):
                    if (params.size() == 1)
                        return asin(params[0]);
                    break;
                case do_hash("acos"):
                    if (params.size() == 1)
                        return acos(params[0]);
                    break;
                case do_hash("sinh"):
                    if (params.size() == 1)
                        return sinh(params[0]);
                    break;
                case do_hash("cosh"):
                    if (params.size() == 1)
                        return cosh(params[0]);
                    break;
                case do_hash("tanh"):
                    if (params.size() == 1)
                        return tanh(params[0]);
                    break;
                case do_hash("gamma"):
                    if (params.size() == 1)
                        return tgamma(params[0]);
                    break;
                case do_hash("isslimechunk"):
                    if (params.size() == 2) {
                        int x = static_cast<int>(round(params[0]));
                        int z = static_cast<int>(round(params[1]));
                        auto seed = (x * 0x1f1f1f1fu) ^ (uint32_t)z;
                        std::mt19937 mt(seed);
                        return mt() % 10 == 0;
                    }
                    break;
                case do_hash("sum"):
                    return std::accumulate(params.begin(), params.end(), 0.0);
                    break;
                case do_hash("min"):
                    return *min_element(params.begin(), params.end());
                    break;
                case do_hash("max"):
                    return *max_element(params.begin(), params.end());
                    break;
                case do_hash("id"):
                    if (blockdataInitialized) {
                        if (params.size() == 0) {
                            return blockSource->getBlock(here).getId();
                        } else if (params.size() == 3) {
                            return blockSource
                                ->getBlock(
                                    here +
                                    BlockPos(
                                        static_cast<int>(floor(params[0])),
                                        static_cast<int>(floor(params[1])),
                                        static_cast<int>(floor(params[2]))))
                                .getId();
                        }
                    }
                    return 0;
                    break;
                case do_hash("data"):
                    if (blockdataInitialized) {
                        if (params.size() == 0) {
                            return (const_cast<Block&>(
                                        blockSource->getBlock(here)))
                                .getTileData();
                        } else if (params.size() == 3) {
                            return (const_cast<Block&>(blockSource->getBlock(
                                        here +
                                        BlockPos(
                                            static_cast<int>(floor(params[0])),
                                            static_cast<int>(floor(params[1])),
                                            static_cast<int>(
                                                floor(params[2]))))))
                                .getTileData();
                        }
                    }
                    return 0;
                    break;
                case do_hash("issolid"):
                    if (blockdataInitialized) {
                        if (params.size() == 0) {
                            return blockSource->getBlock(here).isSolid();
                        } else if (params.size() == 3) {
                            return blockSource
                                ->getBlock(
                                    here +
                                    BlockPos(
                                        static_cast<int>(floor(params[0])),
                                        static_cast<int>(floor(params[1])),
                                        static_cast<int>(floor(params[2]))))
                                .isSolid();
                        }
                    }
                    return 0;
                    break;
                case do_hash("iswaterblocking"):
                    if (blockdataInitialized) {
                        if (params.size() == 0) {
                            return blockSource->getBlock(here)
                                .isWaterBlocking();
                        } else if (params.size() == 3) {
                            return blockSource
                                ->getBlock(
                                    here +
                                    BlockPos(
                                        static_cast<int>(floor(params[0])),
                                        static_cast<int>(floor(params[1])),
                                        static_cast<int>(floor(params[2]))))
                                .isWaterBlocking();
                        }
                    }
                    return 0;
                    break;
                case do_hash("issbblock"):
                    if (blockdataInitialized) {
                        if (params.size() == 0) {
                            return !blockSource->getBlock(here)
                                        .isSolidBlockingBlock();
                        } else if (params.size() == 3) {
                            return !blockSource
                                        ->getBlock(
                                            here +
                                            BlockPos(static_cast<int>(
                                                         floor(params[0])),
                                                     static_cast<int>(
                                                         floor(params[1])),
                                                     static_cast<int>(
                                                         floor(params[2]))))
                                        .isSolidBlockingBlock();
                        }
                    }
                    return 0;
                    break;
                case do_hash("issurface"):
                    if (blockdataInitialized) {
                        BlockPos tmp = here;
                        if (params.size() == 0) {
                            tmp = tmp + BlockPos(0, 1, 0);
                        } else if (params.size() == 3) {
                            tmp =
                                tmp +
                                BlockPos(static_cast<int>(floor(params[0])),
                                         static_cast<int>(floor(params[1])) + 1,
                                         static_cast<int>(floor(params[2])));
                        }
                        return blockSource->getHeightmapPos(tmp) == tmp;
                    }
                    return 0;
                    break;
                default:
                    if (blockdataInitialized) {
                        std::string sname(name);
                        if (sname.substr(0, 3) == "is_") {
                            if (sname.find(":") == std::string::npos) {
                                sname = "minecraft:" + sname.substr(3);
                            } else {
                                sname = sname.substr(3);
                            }
                            if (params.size() == 0) {
                                if (sname ==
                                    blockSource->getBlock(here).getTypeName()) {
                                    return 1;
                                }
                                return 0;
                            } else {
                                if (sname ==
                                    blockSource
                                        ->getBlock(
                                            here +
                                            BlockPos(static_cast<int>(
                                                         floor(params[0])),
                                                     static_cast<int>(
                                                         floor(params[1])),
                                                     static_cast<int>(
                                                         floor(params[2]))))
                                        .getTypeName()) {
                                    return 1;
                                }
                                return 0;
                            }
                        } else if (sname.substr(0, 4) == "has_") {
                            sname = sname.substr(4);
                            if (params.size() == 0) {
                                if (blockSource->getBlock(here)
                                        .getTypeName()
                                        .find(sname) != std::string::npos) {
                                    return 1;
                                }
                                return 0;
                            } else {
                                if (blockSource
                                        ->getBlock(
                                            here +
                                            BlockPos(static_cast<int>(
                                                         floor(params[0])),
                                                     static_cast<int>(
                                                         floor(params[1])),
                                                     static_cast<int>(
                                                         floor(params[2]))))
                                        .getTypeName()
                                        .find(sname) != std::string::npos) {
                                    return 1;
                                }
                                return 0;
                            }
                        }
                        return 0;
                    }
            }
            return 0;
        }
    };
}  // namespace worldedit
#endif