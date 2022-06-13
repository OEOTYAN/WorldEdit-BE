//
// Created by OEOTYAN on 2021/2/8.
//
#pragma once
#ifndef WORLDEDIT_CONVEXREGION_H
#define WORLDEDIT_CONVEXREGION_H

#include "pch.h"
#include "Region.h"
namespace worldedit {
    class Triangle;
    class Edge {
       public:
        Vec3 start;
        Vec3 end;

        explicit Edge(const Vec3& _start, const Vec3& _end) : start(_start), end(_end){};
        bool     operator==(const Edge& other) const;
        Triangle createTriangle(const Vec3&);
    };
    class _hash {
       public:
        size_t operator()(const Edge& rc) const { return std::hash<Vec3>()(rc.start) ^ std::hash<Vec3>()(rc.end); }
    };
    class Triangle {
       public:
        Vec3  vertices[3];
        Vec3  normal;
        float maxDotProduct;

        Triangle() = default;
        Triangle(const Vec3& v0, const Vec3& v1, const Vec3& v2);
        Vec3 getVertex(const int& index) const { return vertices[index]; };
        Edge getEdge(const int& index);
        bool below(const Vec3& pt) { return normal.dot(pt) < maxDotProduct; }
        bool above(const Vec3& pt) { return normal.dot(pt) > maxDotProduct; }
        bool operator==(const Triangle& v) const;
    };

    class Region;

    class ConvexRegion : public Region {
       private:
        bool addVertex(const BlockPos& vertex);

        bool containsRaw(const Vec3& pt);

        void updateEdges();

        std::unordered_set<BlockPos> vertices;

        std::vector<Triangle> triangles;

        std::unordered_set<Edge, _hash> edges;

        std::unordered_set<BlockPos> vertexBacklog;

        BlockPos centerAccum;

        Triangle lastTriangle;

        bool hasLast = false;

       public:
        std::vector<BlockPos> poss;

        void updateBoundingBox() override;

        explicit ConvexRegion(const BoundingBox& region, const int& dim);

        int size() const override;

        void renderRegion() override;

        std::pair<std::string, bool> shift(const BlockPos& change) override;

        Vec3 getCenter() const override {
            return centerAccum.toVec3() * (1.0f / vertices.size()) + Vec3(0.5, 0.5, 0.5);
        };

        void forEachLine(const std::function<void(const BlockPos&, const BlockPos&)>& todo) override;

        bool setMainPos(const BlockPos& pos, const int& dim) override;

        bool setVicePos(const BlockPos& pos, const int& dim) override;

        bool contains(const BlockPos& pos) override;
    };
}  // namespace worldedit

#endif  // WORLDEDIT_CONVEXREGION_H