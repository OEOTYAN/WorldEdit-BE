//
// Created by OEOTYAN on 2022/06/10.
//
#include "Eval.h"
#include <MC/LevelChunk.hpp>
#include <MC/Brightness.hpp>
#include <MC/Dimension.hpp>
#include <MC/Player.hpp>
#include "FastNoiseLite.h"
// #include <MC/ChunkBlockPos.hpp>

#define tryGetParameter(size, params, n, f, t) \
    if (size >= n) {                           \
        f(static_cast<t>(params[n - 1]));      \
    }
namespace worldedit {
    double posfmod(double x, double y) {
        return x - floor(x / y) * y;
    }
    int getHighestTerrainBlock(BlockSource* blockSource, int x, int z, int minY, int maxY, std::string filter) {
        EvalFunctions f;
        f.setbs(blockSource);
        // auto* chunk = blockSource->getChunkAt(BlockPos(x, maxY, z));
        std::unordered_map<std::string, double> variables;
        for (int y = maxY; y >= minY; y--) {
            BlockPos pos(x, y, z);
            auto* block = &blockSource->getBlock(pos);
            if (block != BedrockBlocks::mAir) {
                if (filter == "") {
                    return y;
                } else {
                    f.setPos(pos);
                    variables["rx"] = pos.x;
                    variables["ry"] = pos.y;
                    variables["rz"] = pos.z;
                    if (cpp_eval::eval<double>(filter, variables, f) > 0.5) {
                        return y;
                    }
                }
            }
        }
        return minY - 1;
    }

    void EvalFunctions::setbox(BoundingBox box) {
        normalSearchBox = box;
        normalSearchBox.min -= normalSearchDis;
        normalSearchBox.max += normalSearchDis;
        size = normalSearchBox.max - normalSearchBox.min + 1;
        posMap.clear();
        solidMap.clear();
        searchBoxInitialized = true;
    }

    void EvalFunctions::buildSearchCache() {
        if (!searchBoxInitialized || searchCacheBuilded) {
            return;
        }
        try {
            posMap.resize(size.x * size.y * size.z + 1);
            solidMap = std::vector<long long>(size.x * size.y * size.z + 1, 0);
        } catch (std::bad_alloc) {
            Level::broadcastText("Out of memory", TextType::RAW);
            return;
        }
        posMap[size.x * size.y * size.z] = {0, 0, 0};
        solidMap[size.x * size.y * size.z] = 0;

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
        for (int x = 1; x < size.x; ++x)
            for (int y = 0; y < size.y; ++y)
                for (int z = 0; z < size.z; ++z) {
                    long long index = (y + size.y * z) * size.x + x;
                    long long indexf = (y + size.y * z) * size.x + (x - 1);
                    solidMap[index] = solidMap[index] + solidMap[indexf];
                    posMap[index].x = posMap[index].x + posMap[indexf].x;
                    posMap[index].y = posMap[index].y + posMap[indexf].y;
                    posMap[index].z = posMap[index].z + posMap[indexf].z;
                }
        for (int y = 1; y < size.y; ++y)
            for (int x = 0; x < size.x; ++x)
                for (int z = 0; z < size.z; ++z) {
                    long long index = (y + size.y * z) * size.x + x;
                    long long indexf = ((y - 1) + size.y * z) * size.x + x;
                    solidMap[index] = solidMap[index] + solidMap[indexf];
                    posMap[index].x = posMap[index].x + posMap[indexf].x;
                    posMap[index].y = posMap[index].y + posMap[indexf].y;
                    posMap[index].z = posMap[index].z + posMap[indexf].z;
                }
        for (int z = 1; z < size.z; ++z)
            for (int x = 0; x < size.x; ++x)
                for (int y = 0; y < size.y; ++y) {
                    long long index = (y + size.y * z) * size.x + x;
                    long long indexf = (y + size.y * (z - 1)) * size.x + x;
                    solidMap[index] = solidMap[index] + solidMap[indexf];
                    posMap[index].x = posMap[index].x + posMap[indexf].x;
                    posMap[index].y = posMap[index].y + posMap[indexf].y;
                    posMap[index].z = posMap[index].z + posMap[indexf].z;
                }
        searchCacheBuilded = true;
    }
    long long EvalFunctions::getIndex(const BlockPos& pos) {
        auto localPos = pos - normalSearchBox.min;
        if (pos.x < normalSearchBox.min.x || pos.y < normalSearchBox.min.y || pos.z < normalSearchBox.min.z ||
            pos.x > normalSearchBox.max.x || pos.y > normalSearchBox.max.y || pos.z > normalSearchBox.max.z)
            return size.y * size.z * size.x;
        return (localPos.y + size.y * localPos.z) * size.x + localPos.x;
    }
    long long EvalFunctions::getSolidMap(const BlockPos& pos1, const BlockPos& pos2) {
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
    LongLong3 EvalFunctions::getPosMap(const BlockPos& pos1, const BlockPos& pos2) {
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
    double EvalFunctions::operator()(std::string const& name, const std::vector<double>& params) {
        auto size = params.size();
        BlockPos tmp = here;
        if (size == 3) {
            tmp += BlockPos(static_cast<int>(floor(params[0])), static_cast<int>(floor(params[1])),
                            static_cast<int>(floor(params[2])));
        }
        switch (do_hash(name.c_str())) {
            case do_hash("rand"):
                if (size == 0) {
                    return RNG::rand<double>();
                } else if (size == 2) {
                    return RNG::rand<double>(params[0], params[1]);
                }
                break;
            case do_hash("saturate"):
                if (size == 3)
                    return std::min(1.0, std::max(0.0, params[0]));
                break;
            case do_hash("clamp"):
                if (size == 3)
                    return std::min(params[2], std::max(params[1], params[0]));
                break;
            case do_hash("lerp"):
                if (size == 3)
                    return params[0] * (1 - params[2]) + params[1] * params[2];
                break;
            case do_hash("mod"):
                if (size == 2)
                    return posfmod(params[0], params[1]);
                break;
            case do_hash("abs"):
                if (size == 1)
                    return abs(params[0]);
                break;
            case do_hash("sin"):
                if (size == 1)
                    return sin(params[0]);
                break;
            case do_hash("cos"):
                if (size == 1)
                    return cos(params[0]);
                break;
            case do_hash("sign"):
                if (size == 1)
                    return params[0] == 0.0 ? 0.0 : (params[0] > 0.0 ? 1.0 : -1.0);
                break;
            case do_hash("log10"):
            case do_hash("lg"):
                if (size == 1)
                    return log10(params[0]);
                break;
            case do_hash("ln"):
                if (size == 1)
                    return log(params[0]);
                break;
            case do_hash("log2"):
                if (size == 1)
                    return log2(params[0]);
                break;
            case do_hash("round"):
                if (size == 1)
                    return round(params[0]);
                break;
            case do_hash("floor"):
                if (size == 1)
                    return floor(params[0]);
                break;
            case do_hash("ceil"):
                if (size == 1)
                    return ceil(params[0]);
                break;
            case do_hash("exp"):
                if (size == 1)
                    return exp(params[0]);
                break;
            case do_hash("exp2"):
                if (size == 1)
                    return exp2(params[0]);
                break;
            case do_hash("sqrt"):
                if (size == 1)
                    return sqrt(params[0]);
                break;
            case do_hash("tan"):
                if (size == 1)
                    return tan(params[0]);
                break;
            case do_hash("atan"):
                if (size == 1)
                    return atan(params[0]);
                break;
            case do_hash("atan2"):
                if (size == 2)
                    return atan2(params[0], params[1]);
                break;
            case do_hash("asin"):
                if (size == 1)
                    return asin(params[0]);
                break;
            case do_hash("acos"):
                if (size == 1)
                    return acos(params[0]);
                break;
            case do_hash("sinh"):
                if (size == 1)
                    return sinh(params[0]);
                break;
            case do_hash("cosh"):
                if (size == 1)
                    return cosh(params[0]);
                break;
            case do_hash("tanh"):
                if (size == 1)
                    return tanh(params[0]);
                break;
            case do_hash("gamma"):
                if (size == 1)
                    return tgamma(params[0]);
                break;
            case do_hash("isslimechunk"):
                if (size == 2) {
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
                    return blockSource->getBlock(tmp).getId();
                }
                return 0;
                break;
            case do_hash("runtimeid"):
                if (blockdataInitialized) {
                    return blockSource->getBlock(tmp).getRuntimeId();
                }
                return 0;
                break;
            case do_hash("hsa"):
                if (blockdataInitialized) {
                    for (auto& hsa : blockSource->getChunkAt(tmp)->getSpawningAreas()) {
                        if (((hsa.aabb.max.x - hsa.aabb.min.x + 1) / 2 + hsa.aabb.min.x == tmp.x) &&
                            ((hsa.aabb.max.z - hsa.aabb.min.z + 1) / 2 + hsa.aabb.min.z == tmp.z) &&
                            tmp.y >= hsa.aabb.min.y) {
                            if (hsa.type == LevelChunk::HardcodedSpawnAreaType::SWAMP_HUT ||
                                hsa.type == LevelChunk::HardcodedSpawnAreaType::PILLAGER_OUTPOST) {
                                if (tmp.y <= hsa.aabb.max.y - 3) {
                                    return static_cast<int>(hsa.type);
                                }
                            } else {
                                if (tmp.y <= hsa.aabb.max.y) {
                                    return static_cast<int>(hsa.type);
                                }
                            }
                        }
                    }
                    return 0;
                }
                return 0;
                break;
            case do_hash("biome"):
                if (blockdataInitialized) {
                    return blockSource->getConstBiome(tmp).getId();
                }
                return 0;
                break;
            case do_hash("hasplayer"):
                if (blockdataInitialized) {
                    auto& dimension = blockSource->getDimensionConst();
                    if (size == 1) {
                        return nullptr != dimension.fetchAnyPlayer(here.toVec3(), static_cast<float>(params[0]));
                    } else if (size == 4) {
                        return nullptr != dimension.fetchAnyPlayer(here.toVec3() +Vec3(params[1], params[2], params[3]),
                                                                   static_cast<float>(params[0]));
                    }
                }
                return 0;
                break;
            case do_hash("noplayer"):
                if (blockdataInitialized) {
                    auto& dimension = blockSource->getDimensionConst();
                    Vec3 tmpVec;
                    if (size == 1) {
                        tmpVec = here.toVec3();
                    } else if (size == 4) {
                        tmpVec = here.toVec3()+ Vec3(params[1], params[2], params[3]);
                    }
                    float playerDistance = FLT_MAX;
                    dimension.forEachPlayer([&](Player& player) {
                        float dis = player.getPosition().distanceToSqr(tmpVec);
                        playerDistance = std::min(dis, playerDistance);
                        return true;
                    });
                    return playerDistance == FLT_MAX || playerDistance >= pow2(static_cast<float>(params[0]));
                }
                return 0;
                break;
            case do_hash("hasuntickedchunk"):
                if (blockdataInitialized) {
                    if (size == 1) {
                        return blockSource->hasUntickedNeighborChunk(ChunkPos(here), static_cast<int>(params[0]));
                    } else if (size == 4) {
                        return blockSource->hasUntickedNeighborChunk(
                            ChunkPos(here + BlockPos(static_cast<int>(floor(params[1])),
                                                     static_cast<int>(floor(params[2])),
                                                     static_cast<int>(floor(params[3])))),
                            static_cast<int>(params[0]));
                    }
                }
                return 0;
            case do_hash("chunksfullyloaded"):
                if (blockdataInitialized) {
                    if (size == 1) {
                        return blockSource->areChunksFullyLoaded(here, static_cast<int>(params[0]));
                    } else if (size == 4) {
                        return blockSource->areChunksFullyLoaded(
                            (here + BlockPos(static_cast<int>(floor(params[1])), static_cast<int>(floor(params[2])),
                                             static_cast<int>(floor(params[3])))),
                            static_cast<int>(params[0]));
                    }
                }
                return 0;
                break;
            case do_hash("data"):
                if (blockdataInitialized) {
                    return (const_cast<Block&>(blockSource->getBlock(tmp))).getTileData();
                }
                return 0;
                break;
            case do_hash("issolid"):
                if (blockdataInitialized) {
                    return blockSource->getBlock(tmp).isSolid();
                }
                return 0;
                break;
            case do_hash("iswaterblocking"):
                if (blockdataInitialized) {
                    return blockSource->getBlock(tmp).isWaterBlocking();
                }
                return 0;
                break;
            case do_hash("issbblock"):
                if (blockdataInitialized) {
                    return blockSource->getBlock(tmp).isSolidBlockingBlock();
                }
                return 0;
                break;
            case do_hash("istop"):
                if (blockdataInitialized) {
                    return blockSource->getHeightmapPos(tmp) == tmp;
                }
                return 0;
                break;
            case do_hash("destroyspeed"):
                if (blockdataInitialized) {
                    return blockSource->getBlock(tmp).getDestroySpeed();
                }
                return 0;
                break;
            case do_hash("thickness"):
                if (blockdataInitialized) {
                    return blockSource->getBlock(tmp).getThickness();
                }
                return 0;
                break;
            case do_hash("translucency"):
                if (blockdataInitialized) {
                    return blockSource->getBlock(tmp).getTranslucency();
                }
                return 0;
                break;
            case do_hash("light"):
                if (blockdataInitialized) {
                    return blockSource->getBlock(tmp).getLight().value;
                }
                return 0;
                break;
            case do_hash("emissive"):
                if (blockdataInitialized) {
                    return blockSource->getBlock(tmp).getLightEmission().value;
                }
                return 0;
                break;
            case do_hash("normalx"):
                if (searchBoxInitialized) {
                    if (size == 0) {
                        buildSearchCache();
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
                    if (size == 0) {
                        buildSearchCache();
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
                    if (size == 0) {
                        buildSearchCache();
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
                    if (size == 0) {
                        buildSearchCache();
                        auto solid = getSolidMap(here - normalSearchDis, here + normalSearchDis);
                        auto posSum = getPosMap(here - normalSearchDis, here + normalSearchDis);
                        double sumx = 0;
                        double sumy = 0;
                        double sumz = 0;
                        sumx = static_cast<double>(solid * here.x - posSum.x);
                        sumy = static_cast<double>(solid * here.y - posSum.y);
                        sumz = static_cast<double>(solid * here.z - posSum.z);
                        double length = sqrt(sumx * sumx + sumy * sumy + sumz * sumz);
                        return acos(sumy / length) / M_PI * 180.0;
                    }
                }
                return 0;
                break;
            case do_hash("issurface"):
                if (blockdataInitialized) {
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
                    buildSearchCache();
                    return 1.0 - static_cast<double>(getSolidMap(tmp - normalSearchDis, tmp + normalSearchDis)) /
                                     static_cast<double>((normalSearchDis * 2 + 1) * normalSearchDis * 2 + 1);
                }
                return 0;
                break;

            case do_hash("simplex"):
                if (size >= 3) {
                    FastNoiseLite noise = FastNoiseLite();
                    noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Simplex);
                    tryGetParameter(size, params, 4, noise.SetSeed, int64_t);
                    tryGetParameter(size, params, 5, noise.SetFractalType, FastNoiseLite::FractalType);
                    tryGetParameter(size, params, 6, noise.SetFractalOctaves, int64_t);
                    tryGetParameter(size, params, 7, noise.SetFractalLacunarity, double);
                    tryGetParameter(size, params, 8, noise.SetFractalGain, double);
                    tryGetParameter(size, params, 9, noise.SetFractalWeightedStrength, double);
                    tryGetParameter(size, params, 10, noise.SetFractalPingPongStrength, double);
                    return (noise.GetNoise(params[0], params[1], params[2]) + 1.0) * 0.5;
                }
                return 0;
                break;

            case do_hash("perlin"):
                if (size >= 3) {
                    FastNoiseLite noise = FastNoiseLite();
                    noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
                    tryGetParameter(size, params, 4, noise.SetSeed, int64_t);
                    tryGetParameter(size, params, 5, noise.SetFractalType, FastNoiseLite::FractalType);
                    tryGetParameter(size, params, 6, noise.SetFractalOctaves, int64_t);
                    tryGetParameter(size, params, 7, noise.SetFractalLacunarity, double);
                    tryGetParameter(size, params, 8, noise.SetFractalGain, double);
                    tryGetParameter(size, params, 9, noise.SetFractalWeightedStrength, double);
                    tryGetParameter(size, params, 10, noise.SetFractalPingPongStrength, double);
                    return (noise.GetNoise(params[0], params[1], params[2]) + 1.0) * 0.5;
                }
                return 0;
                break;

            case do_hash("cubic"):
                if (size >= 3) {
                    FastNoiseLite noise = FastNoiseLite();
                    noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_ValueCubic);
                    tryGetParameter(size, params, 4, noise.SetSeed, int64_t);
                    tryGetParameter(size, params, 5, noise.SetFractalType, FastNoiseLite::FractalType);
                    tryGetParameter(size, params, 6, noise.SetFractalOctaves, int64_t);
                    tryGetParameter(size, params, 7, noise.SetFractalLacunarity, double);
                    tryGetParameter(size, params, 8, noise.SetFractalGain, double);
                    tryGetParameter(size, params, 9, noise.SetFractalWeightedStrength, double);
                    tryGetParameter(size, params, 10, noise.SetFractalPingPongStrength, double);
                    return (noise.GetNoise(params[0], params[1], params[2]) + 1.0) * 0.5;
                }
                return 0;
                break;

            case do_hash("value"):
                if (size >= 3) {
                    FastNoiseLite noise = FastNoiseLite();
                    noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Value);
                    tryGetParameter(size, params, 4, noise.SetSeed, int64_t);
                    tryGetParameter(size, params, 5, noise.SetFractalType, FastNoiseLite::FractalType);
                    tryGetParameter(size, params, 6, noise.SetFractalOctaves, int64_t);
                    tryGetParameter(size, params, 7, noise.SetFractalLacunarity, double);
                    tryGetParameter(size, params, 8, noise.SetFractalGain, double);
                    tryGetParameter(size, params, 9, noise.SetFractalWeightedStrength, double);
                    tryGetParameter(size, params, 10, noise.SetFractalPingPongStrength, double);
                    return (noise.GetNoise(params[0], params[1], params[2]) + 1.0) * 0.5;
                }
                return 0;
                break;

            case do_hash("voronoi"):
                if (size >= 3) {
                    FastNoiseLite noise = FastNoiseLite();
                    noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Cellular);
                    tryGetParameter(size, params, 4, noise.SetSeed, int64_t);
                    tryGetParameter(size, params, 5, noise.SetCellularReturnType, FastNoiseLite::CellularReturnType);
                    tryGetParameter(size, params, 6, noise.SetCellularDistanceFunction,
                                    FastNoiseLite::CellularDistanceFunction);
                    tryGetParameter(size, params, 7, noise.SetCellularJitter, double);
                    tryGetParameter(size, params, 8, noise.SetFractalType, FastNoiseLite::FractalType);
                    tryGetParameter(size, params, 9, noise.SetFractalOctaves, int64_t);
                    tryGetParameter(size, params, 10, noise.SetFractalLacunarity, double);
                    tryGetParameter(size, params, 11, noise.SetFractalGain, double);
                    tryGetParameter(size, params, 12, noise.SetFractalWeightedStrength, double);
                    tryGetParameter(size, params, 13, noise.SetFractalPingPongStrength, double);
                    return (noise.GetNoise(params[0], params[1], params[2]) + 1.0) * 0.5;
                }
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
                        return sname == blockSource->getBlock(tmp).getTypeName();
                    } else if (sname.substr(0, 4) == "has_") {
                        sname = sname.substr(4);
                        return blockSource->getBlock(tmp).getTypeName().find(sname) != std::string::npos;
                    }
                    return 0;
                }
        }
        return 0;
    }

}  // namespace worldedit
