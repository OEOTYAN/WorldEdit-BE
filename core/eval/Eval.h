#pragma once
//
// Created by OEOTYAN on 2022/5/15.
//
#ifndef WORLDEDIT_EVAL_H
#define WORLDEDIT_EVAL_H

#include "pch.h"
#include <MC/Biome.hpp>
// #include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BedrockBlocks.hpp>
#include <MC/BlockSource.hpp>
#include <MC/Level.hpp>
// #include <MC/Player.hpp>
// #include <MC/ServerPlayer.hpp>
// #include <MC/Dimension.hpp>
// #include <MC/ItemStack.hpp>
// #include "string/StringTool.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
// #include <ScheduleAPI.h>
// #include <DynamicCommandAPI.h>
#include "CppEval.h"
#include "FastNoiseLite.h"

#ifndef __M__PI__
#define __M__PI__ 3.141592653589793238462643383279
#endif

namespace worldedit {

    double posfmod(double x, double y);

    int getHighestTerrainBlock(class BlockSource* blockSource, int x, int z, int minY, int maxY, std::string filter);

    class LongLong3 {
       public:
        long long x = 0;
        long long y = 0;
        long long z = 0;

        constexpr LongLong3& operator+=(LongLong3 const& b) {
            x += b.x;
            y += b.y;
            z += b.z;
            return *this;
        }

        constexpr LongLong3& operator-=(LongLong3 const& b) {
            x -= b.x;
            y -= b.y;
            z -= b.z;
            return *this;
        }
    };
    double uniformRandDouble();
    class EvalFunctions {
        int normalSearchDis = 4;
        BlockPos here;
        BlockPos size;
        BlockSource* blockSource;
        BoundingBox normalSearchBox;
        std::vector<LongLong3> posMap;
        std::vector<long long> solidMap;
        bool searchBoxInitialized = false;
        bool blockdataInitialized = false;

       public:
        void setPos(const BlockPos& pos) { here = pos; }
        void setbs(BlockSource* bs) {
            blockSource = bs;
            blockdataInitialized = true;
        }
        void setbox(BoundingBox box) {
            normalSearchBox = box;
            normalSearchBox.min -= normalSearchDis;
            normalSearchBox.max += normalSearchDis;
            size = normalSearchBox.max - normalSearchBox.min + 1;
            posMap.clear();
            solidMap.clear();
            try {
                posMap.resize(size.x * size.y * size.z + 1);
                solidMap = std::vector<long long>(size.x * size.y * size.z + 1, 0);
            } catch (std::bad_alloc) {
                Level::broadcastText("Out of memory", TextType::RAW);
                return;
            }
            posMap[size.x * size.y * size.z] = {0, 0, 0};
            solidMap[size.x * size.y * size.z] = 0;
            searchBoxInitialized = true;
            normalSearchBox.forEachBlockInBox([&](const BlockPos& pos) {
                if (&blockSource->getBlock(pos) == BedrockBlocks::mAir) {
                    return;
                }
                auto index = getIndex(pos);
                int counts = 0;
                for (auto& calPos : pos.getNeighbors()) {
                    counts += &blockSource->getBlock(calPos) != BedrockBlocks::mAir;
                }
                solidMap[index] = counts == 6;
                if (counts == 6) {
                    posMap[index].x = pos.x;
                    posMap[index].y = pos.y;
                    posMap[index].z = pos.z;
                }
            });
            for (int x = 1; x < size.x; x++)
                for (int y = 0; y < size.y; y++)
                    for (int z = 0; z < size.z; z++) {
                        long long index = (y + size.y * z) * size.x + x;
                        long long indexf = (y + size.y * z) * size.x + (x - 1);
                        solidMap[index] = solidMap[index] + solidMap[indexf];
                        posMap[index].x = posMap[index].x + posMap[indexf].x;
                        posMap[index].y = posMap[index].y + posMap[indexf].y;
                        posMap[index].z = posMap[index].z + posMap[indexf].z;
                    }
            for (int y = 1; y < size.y; y++)
                for (int x = 0; x < size.x; x++)
                    for (int z = 0; z < size.z; z++) {
                        long long index = (y + size.y * z) * size.x + x;
                        long long indexf = ((y - 1) + size.y * z) * size.x + x;
                        solidMap[index] = solidMap[index] + solidMap[indexf];
                        posMap[index].x = posMap[index].x + posMap[indexf].x;
                        posMap[index].y = posMap[index].y + posMap[indexf].y;
                        posMap[index].z = posMap[index].z + posMap[indexf].z;
                    }
            for (int z = 1; z < size.z; z++)
                for (int x = 0; x < size.x; x++)
                    for (int y = 0; y < size.y; y++) {
                        long long index = (y + size.y * z) * size.x + x;
                        long long indexf = (y + size.y * (z - 1)) * size.x + x;
                        solidMap[index] = solidMap[index] + solidMap[indexf];
                        posMap[index].x = posMap[index].x + posMap[indexf].x;
                        posMap[index].y = posMap[index].y + posMap[indexf].y;
                        posMap[index].z = posMap[index].z + posMap[indexf].z;
                    }
        }
        long long getIndex(const BlockPos& pos) {
            auto localPos = pos - normalSearchBox.min;
            if (pos.x < normalSearchBox.min.x || pos.y < normalSearchBox.min.y || pos.z < normalSearchBox.min.z ||
                pos.x > normalSearchBox.max.x || pos.y > normalSearchBox.max.y || pos.z > normalSearchBox.max.z)
                return size.y * size.z * size.x;
            return (localPos.y + size.y * localPos.z) * size.x + localPos.x;
        }
        long long getSolidMap(const BlockPos& pos1, const BlockPos& pos2) {
            long long res = 0;
            res = solidMap[getIndex(pos2)];
            res -= solidMap[getIndex({pos1.x - 1, pos2.y, pos2.z})];
            res -= solidMap[getIndex({pos2.x, pos1.y - 1, pos2.z})];
            res -= solidMap[getIndex({pos2.x, pos2.y, pos1.z - 1})];
            res += solidMap[getIndex({pos2.x, pos1.y - 1, pos1.z - 1})];
            res += solidMap[getIndex({pos1.x - 1, pos2.y, pos1.z - 1})];
            res += solidMap[getIndex({pos1.x - 1, pos1.y - 1, pos2.z})];
            res -= solidMap[getIndex(pos1 - 1)];
            return res;
        }
        LongLong3 getPosMap(const BlockPos& pos1, const BlockPos& pos2) {
            LongLong3 res;
            res = posMap[getIndex(pos2)];
            res -= posMap[getIndex({pos1.x - 1, pos2.y, pos2.z})];
            res -= posMap[getIndex({pos2.x, pos1.y - 1, pos2.z})];
            res -= posMap[getIndex({pos2.x, pos2.y, pos1.z - 1})];
            res += posMap[getIndex({pos2.x, pos1.y - 1, pos1.z - 1})];
            res += posMap[getIndex({pos1.x - 1, pos2.y, pos1.z - 1})];
            res += posMap[getIndex({pos1.x - 1, pos1.y - 1, pos2.z})];
            res -= posMap[getIndex(pos1 - 1)];
            return res;
        }
        double operator()(std::string const& name, const std::vector<double>& params) {
            switch (do_hash(name.c_str())) {
                case do_hash("rand"):
                    if (params.size() == 0) {
                        return uniformRandDouble();
                    } else if (params.size() == 2) {
                        return uniformRandDouble() * (params[1] - params[0]) + params[0];
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
                        return params[0] == 0.0 ? 0.0 : (params[0] > 0.0 ? 1.0 : -1.0);
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
                                ->getBlock(here + BlockPos(static_cast<int>(floor(params[0])),
                                                           static_cast<int>(floor(params[1])),
                                                           static_cast<int>(floor(params[2]))))
                                .getId();
                        }
                    }
                    return 0;
                    break;
                case do_hash("biome"):
                    if (blockdataInitialized) {
                        if (params.size() == 0) {
                            return blockSource->getConstBiome(here).getId();
                        } else if (params.size() == 3) {
                            return blockSource
                                ->getConstBiome(here + BlockPos(static_cast<int>(floor(params[0])),
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
                            return (const_cast<Block&>(blockSource->getBlock(here))).getTileData();
                        } else if (params.size() == 3) {
                            return (const_cast<Block&>(
                                        blockSource->getBlock(here + BlockPos(static_cast<int>(floor(params[0])),
                                                                              static_cast<int>(floor(params[1])),
                                                                              static_cast<int>(floor(params[2]))))))
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
                                ->getBlock(here + BlockPos(static_cast<int>(floor(params[0])),
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
                            return blockSource->getBlock(here).isWaterBlocking();
                        } else if (params.size() == 3) {
                            return blockSource
                                ->getBlock(here + BlockPos(static_cast<int>(floor(params[0])),
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
                            return blockSource->getBlock(here).isSolidBlockingBlock();
                        } else if (params.size() == 3) {
                            return blockSource
                                ->getBlock(here + BlockPos(static_cast<int>(floor(params[0])),
                                                           static_cast<int>(floor(params[1])),
                                                           static_cast<int>(floor(params[2]))))
                                .isSolidBlockingBlock();
                        }
                    }
                    return 0;
                    break;
                case do_hash("istop"):
                    if (blockdataInitialized) {
                        BlockPos tmp = here;
                        if (params.size() == 0) {
                            tmp = tmp + BlockPos(0, 1, 0);
                        } else if (params.size() == 3) {
                            tmp = tmp + BlockPos(static_cast<int>(floor(params[0])),
                                                 static_cast<int>(floor(params[1])) + 1,
                                                 static_cast<int>(floor(params[2])));
                        }
                        return blockSource->getHeightmapPos(tmp) == tmp;
                    }
                    return 0;
                    break;
                case do_hash("normalx"):
                    if (searchBoxInitialized) {
                        if (params.size() == 0) {
                            auto solid = getSolidMap(here - normalSearchDis, here + normalSearchDis);
                            auto posSum = getPosMap(here - normalSearchDis, here + normalSearchDis);
                            double sumx = 0;
                            double sumy = 0;
                            double sumz = 0;
                            sumx = static_cast<double>(solid * here.x - posSum.x);
                            sumy = static_cast<double>(solid * here.y - posSum.y);
                            sumz = static_cast<double>(solid * here.z - posSum.z);
                            double length = sqrt(sumx * sumx + sumy * sumy + sumz * sumz);
                            return sumx / length;
                        }
                    }
                    return 0;
                    break;
                case do_hash("normaly"):
                    if (searchBoxInitialized) {
                        if (params.size() == 0) {
                            auto solid = getSolidMap(here - normalSearchDis, here + normalSearchDis);
                            auto posSum = getPosMap(here - normalSearchDis, here + normalSearchDis);
                            double sumx = 0;
                            double sumy = 0;
                            double sumz = 0;
                            sumx = static_cast<double>(solid * here.x - posSum.x);
                            sumy = static_cast<double>(solid * here.y - posSum.y);
                            sumz = static_cast<double>(solid * here.z - posSum.z);
                            double length = sqrt(sumx * sumx + sumy * sumy + sumz * sumz);
                            return sumy / length;
                        }
                    }
                    return 0;
                    break;
                case do_hash("normalz"):
                    if (searchBoxInitialized) {
                        if (params.size() == 0) {
                            auto solid = getSolidMap(here - normalSearchDis, here + normalSearchDis);
                            auto posSum = getPosMap(here - normalSearchDis, here + normalSearchDis);
                            double sumx = 0;
                            double sumy = 0;
                            double sumz = 0;
                            sumx = static_cast<double>(solid * here.x - posSum.x);
                            sumy = static_cast<double>(solid * here.y - posSum.y);
                            sumz = static_cast<double>(solid * here.z - posSum.z);
                            double length = sqrt(sumx * sumx + sumy * sumy + sumz * sumz);
                            return sumz / length;
                        }
                    }
                    return 0;
                    break;
                case do_hash("angle"):
                    if (searchBoxInitialized) {
                        if (params.size() == 0) {
                            auto solid = getSolidMap(here - normalSearchDis, here + normalSearchDis);
                            auto posSum = getPosMap(here - normalSearchDis, here + normalSearchDis);
                            double sumx = 0;
                            double sumy = 0;
                            double sumz = 0;
                            sumx = static_cast<double>(solid * here.x - posSum.x);
                            sumy = static_cast<double>(solid * here.y - posSum.y);
                            sumz = static_cast<double>(solid * here.z - posSum.z);
                            double length = sqrt(sumx * sumx + sumy * sumy + sumz * sumz);
                            return acos(sumy / length) / __M__PI__ * 180.0;
                        }
                    }
                    return 0;
                    break;
                case do_hash("issurface"):
                    if (blockdataInitialized) {
                        BlockPos tmp = here;
                        if (params.size() == 3) {
                            tmp = tmp + BlockPos(static_cast<int>(floor(params[0])), static_cast<int>(floor(params[1])),
                                                 static_cast<int>(floor(params[2])));
                        }
                        if (&blockSource->getBlock(tmp) != BedrockBlocks::mAir) {
                            int counts = 0;
                            for (auto& calPos : tmp.getNeighbors()) {
                                counts += &blockSource->getBlock(calPos) == BedrockBlocks::mAir;
                            }
                            return counts > 0;
                        }
                    }
                    return 0;
                    break;
                case do_hash("issurfacesmooth"):
                    if (searchBoxInitialized) {
                        BlockPos tmp = here;
                        if (params.size() == 3) {
                            tmp = tmp + BlockPos(static_cast<int>(floor(params[0])), static_cast<int>(floor(params[1])),
                                                 static_cast<int>(floor(params[2])));
                        }
                        return 1.0 - static_cast<double>(getSolidMap(tmp - normalSearchDis, tmp + normalSearchDis)) /
                                         static_cast<double>((normalSearchDis * 2 + 1) * normalSearchDis * 2 + 1);
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
                                if (sname == blockSource->getBlock(here).getTypeName()) {
                                    return 1;
                                }
                                return 0;
                            } else {
                                if (sname == blockSource
                                                 ->getBlock(here + BlockPos(static_cast<int>(floor(params[0])),
                                                                            static_cast<int>(floor(params[1])),
                                                                            static_cast<int>(floor(params[2]))))
                                                 .getTypeName()) {
                                    return 1;
                                }
                                return 0;
                            }
                        } else if (sname.substr(0, 4) == "has_") {
                            sname = sname.substr(4);
                            if (params.size() == 0) {
                                if (blockSource->getBlock(here).getTypeName().find(sname) != std::string::npos) {
                                    return 1;
                                }
                                return 0;
                            } else {
                                if (blockSource
                                        ->getBlock(here + BlockPos(static_cast<int>(floor(params[0])),
                                                                   static_cast<int>(floor(params[1])),
                                                                   static_cast<int>(floor(params[2]))))
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