#pragma once

#include "Globals.h"
#include <mc/Biome.hpp>
// #include <mc/BlockInstance.hpp>
#include "CppEval.h"
#include "FastNoiseLite.h"
#include <mc/BedrockBlocks.hpp>
#include <mc/Block.hpp>
#include <mc/BlockSource.hpp>
#include <mc/Level.hpp>

namespace we {

template <class T>
constexpr T binpow(T const& a, uint64_t const& b) {
    if (b == 0) return static_cast<T>(1);
    T res = binpow(a, b / 2);
    if (b % 2) return res * res * a;
    else return res * res;
}

template <class T>
constexpr T pow2(T const& a) {
    return binpow<T>(a, 2);
}

template <class T>
constexpr T pow3(T const& a) {
    return binpow<T>(a, 3);
}

template <class T>
constexpr T pow4(T const& a) {
    return binpow<T>(a, 4);
}

template <class T>
constexpr T pow5(T const& a) {
    return binpow<T>(a, 5);
}

template <class T>
constexpr T pow6(T const& a) {
    return binpow<T>(a, 6);
}

template <class T>
constexpr T pow7(T const& a) {
    return binpow<T>(a, 7);
}

template <class T>
constexpr T pow8(T const& a) {
    return binpow<T>(a, 8);
}

template <class T>
inline T clamp(T const& a, T const& min, T const& max) {
    return std::min(max, std::max(a, min));
}

template <class T>
inline T saturate(T const& a) {
    return std::min(static_cast<T>(1), std::max(a, static_cast<T>(0)));
}

template <class T>
inline T sign(T const& a) {
    if (a == 0) {
        return 0;
    }
    return std::copysign(1, a);
}

template <class T>
inline T signedSqrt(T const& a) {
    return sign(a) * std::sqrt(std::abs(a));
}

double posfmod(double x, double y);

int getHighestTerrainBlock(
    class BlockSource* blockSource,
    int                x,
    int                z,
    int                minY,
    int                maxY,
    std::string        filter
);

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
class EvalFunctions {
    int                    normalSearchDis = 4;
    BlockPos               here;
    BlockPos               size;
    BlockSource*           blockSource;
    BoundingBox            normalSearchBox;
    std::vector<LongLong3> posMap;
    std::vector<long long> solidMap;
    bool                   searchBoxInitialized = false;
    bool                   searchCacheBuilded   = false;
    bool                   blockdataInitialized = false;

public:
    void setPos(BlockPos const& pos) { here = pos; }
    void setbs(BlockSource* bs) {
        blockSource          = bs;
        blockdataInitialized = true;
    }
    void      setbox(BoundingBox box);
    void      buildSearchCache();
    long long getIndex(BlockPos const& pos);
    long long getSolidMap(BlockPos const& pos1, BlockPos const& pos2);
    LongLong3 getPosMap(BlockPos const& pos1, BlockPos const& pos2);
    double    operator()(std::string_view name, const std::vector<double>& params);
};
} // namespace we