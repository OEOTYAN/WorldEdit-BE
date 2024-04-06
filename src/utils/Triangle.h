#pragma once

#include "utils/Math.h"

namespace we {

struct Edge {
public:
    BlockPos start;
    BlockPos end;

    constexpr Edge(BlockPos const& start, BlockPos const& end) : start(start), end(end){};
    constexpr bool operator==(Edge const& other) const {
        return (start == other.start && end == other.end)
            || (end == other.start && start == other.end);
    }
};
inline size_t hash_value(Edge const& rc) {
    size_t seed = std::hash<BlockPos>()(rc.start);
    ll::hash_utils::hashCombine(std::hash<BlockPos>()(rc.start), seed);
    return seed;
}
struct Triangle {
    std::array<BlockPos, 3> vertices;
    Vec3                    normal;
    float                   maxDotProduct;

    float evalMaxDotProduct() const {
        return (float)std::max(
            std::max(normal.dot(vertices[0]), normal.dot(vertices[1])),
            normal.dot(vertices[2])
        );
    }

    Triangle() = default;
    Triangle(BlockPos const& v0, BlockPos const& v1, BlockPos const& v2)
    : vertices({v0, v1, v2}),
      normal(Vec3((v1 - v0).cross(v2 - v0)).normalize()),
      maxDotProduct(evalMaxDotProduct()) {}

    BlockPos const& getVertex(int index) const { return vertices[index]; };

    Edge getEdge(int index) const {
        if (index == 2) return Edge(vertices[index], vertices[0]);
        return Edge(vertices[index], vertices[index + 1]);
    }
    bool below(BlockPos const& pt) const { return normal.dot(pt) < maxDotProduct; }

    bool above(BlockPos const& pt) const { return normal.dot(pt) > maxDotProduct; }

    bool operator==(Triangle const& v) const {
        return (v.maxDotProduct == maxDotProduct) && (v.normal == normal)
            && (v.vertices[0] == vertices[0]) && (v.vertices[1] == vertices[1])
            && (v.vertices[2] == vertices[2]);
    }
};
} // namespace we
