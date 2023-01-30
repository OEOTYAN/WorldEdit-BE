//
// Created by OEOTYAN on 2021/2/8.
//

#include "ConvexRegion.h"
#include "Global.h"
#include "MC/Level.hpp"
#include "MC/Dimension.hpp"
namespace worldedit {
    void ConvexRegion::updateBoundingBox() {
        auto range = Global<Level>->getDimension(dimensionID)->getHeightRange();
        rendertick = 0;
        boundingBox.min = *vertices.begin();
        boundingBox.max = *vertices.begin();

        for (auto& vertice : vertices) {
            boundingBox.min.x = std::min(boundingBox.min.x, vertice.x);
            boundingBox.min.y = std::max(std::min(boundingBox.min.y, vertice.y), static_cast<int>(range.min));
            boundingBox.min.z = std::min(boundingBox.min.z, vertice.z);
            boundingBox.max.x = std::max(boundingBox.max.x, vertice.x);
            boundingBox.max.y = std::min(std::max(boundingBox.max.y, vertice.y), static_cast<int>(range.max) - 1);
            boundingBox.max.z = std::max(boundingBox.max.z, vertice.z);
        }
        for (auto& value : vertexBacklog) {
            boundingBox.min.x = std::min(boundingBox.min.x, value.x);
            boundingBox.min.y = std::max(std::min(boundingBox.min.y, value.y), static_cast<int>(range.min));
            boundingBox.min.z = std::min(boundingBox.min.z, value.z);
            boundingBox.max.x = std::max(boundingBox.max.x, value.x);
            boundingBox.max.y = std::min(std::max(boundingBox.max.y, value.y), static_cast<int>(range.max) - 1);
            boundingBox.max.z = std::max(boundingBox.max.z, value.z);
        }
    }

    void ConvexRegion::updateEdges() {
        edges.clear();
        for (auto triangle : triangles) {
            for (int i = 0; i < 3; ++i) {
                Edge edge = triangle.getEdge(i);
                if (edges.find(edge) == edges.end()) {
                    edges.insert(edge);
                }
            }
        }
    }

    ConvexRegion::ConvexRegion(const BoundingBox& region, const int& dim) : Region(region, dim) {
        vertices.clear();
        poss.clear();
        triangles.clear();
        vertexBacklog.clear();
        edges.clear();
        hasLast = false;
        centerAccum = BlockPos(0, 0, 0);
        this->regionType = CONVEX;
    }

    bool ConvexRegion::containsRaw(const Vec3& pt) {
        if (lastTriangle.above(pt) && hasLast) {
            return false;
        }

        for (Triangle triangle : triangles) {
            if (lastTriangle == triangle) {
                continue;
            }

            if (triangle.above(pt)) {
                hasLast = true;
                lastTriangle = triangle;
                return false;
            }
        }

        return true;
    }

    bool ConvexRegion::addVertex(const BlockPos& vertex) {
        hasLast = false;
        if (vertices.find(vertex) != vertices.end()) {
            return false;
        }
        Vec3 vertexD = vertex.toVec3();
        if (vertices.size() == 3) {
            if (vertexBacklog.find(vertex) != vertexBacklog.end()) {
                return false;
            }
            if (containsRaw(vertexD)) {
                vertexBacklog.insert(vertex);
                updateBoundingBox();
                return true;
            }
        }
        vertices.insert(vertex);
        updateBoundingBox();
        centerAccum = centerAccum + vertex;

        switch (vertices.size()) {
            case 0:
            case 1:
            case 2: {
                return true;
            };
            case 3: {
                std::vector<BlockPos> v;
                v.assign(vertices.begin(), vertices.end());
                triangles.emplace_back(Triangle(v[0].toVec3(), v[1].toVec3(), v[2].toVec3()));
                triangles.emplace_back(Triangle(v[0].toVec3(), v[2].toVec3(), v[1].toVec3()));
                updateEdges();
                return true;
            };
            default:
                break;
        }
        std::unordered_set<Edge, _hash> borderEdges;
        for (auto iter = triangles.begin(); iter != triangles.end();) {
            if ((*iter).above(vertexD)) {
                for (int i = 0; i < 3; ++i) {
                    Edge edge = (*iter).getEdge(i);
                    if (borderEdges.find(edge) == borderEdges.end()) {
                        borderEdges.insert(edge);
                    } else {
                        borderEdges.erase(edge);
                    }
                }
                iter = triangles.erase(iter);
            } else
                ++iter;
        }
        for (Edge edge : borderEdges) {
            triangles.emplace_back(edge.createTriangle(vertexD));
        }

        if (!vertexBacklog.empty()) {
            vertices.erase(vertex);
            std::unordered_set<BlockPos> vertexBacklog2(vertexBacklog);
            vertexBacklog.clear();
            for (BlockPos vertex2 : vertexBacklog2) {
                addVertex(vertex2);
            }
            vertices.insert(vertex);
        }
        updateEdges();
        return true;
    }

    bool ConvexRegion::setMainPos(const BlockPos& pos, const int& dim) {
        dimensionID = dim;
        selecting = 1;
        hasLast = false;
        vertices.clear();
        triangles.clear();
        vertexBacklog.clear();
        edges.clear();
        poss.clear();
        centerAccum = BlockPos(0, 0, 0);
        bool res = addVertex(pos);
        if (res) {
            poss.push_back(pos);
        }
        return res;
    }

    bool ConvexRegion::setVicePos(const BlockPos& pos, const int& dim) {
        if (!selecting || dim != dimensionID) {
            return false;
        }
        bool res = addVertex(pos);
        if (res) {
            poss.push_back(pos);
        }
        return res;
    }

    std::pair<std::string, bool> ConvexRegion::shift(const BlockPos& change) {
        boundingBox.min = boundingBox.min + change;
        boundingBox.max = boundingBox.max + change;
        std::unordered_set<BlockPos> tmpVertices = vertices;
        vertices.clear();
        for (auto vertice : tmpVertices) {
            vertices.insert(vertice + change);
        }
        for (auto& pos : poss) {
            pos += change;
        }
        std::unordered_set<BlockPos> tmpVertexBacklog = vertexBacklog;
        vertexBacklog.clear();
        for (auto vertex : tmpVertexBacklog) {
            vertexBacklog.insert(vertex + change);
        }
        std::vector<Triangle> tmpTriangles = triangles;
        triangles.clear();
        for (auto triangle : tmpTriangles) {
            triangles.emplace_back(Triangle(triangle.vertices[0] + change.toVec3(),
                                            triangle.vertices[1] + change.toVec3(),
                                            triangle.vertices[2] + change.toVec3()));
        }
        std::unordered_set<Edge, _hash> tmpEdges = edges;
        edges.clear();
        for (auto edge : tmpEdges) {
            edges.insert(Edge(edge.start + change.toVec3(), edge.end + change.toVec3()));
        }
        centerAccum = centerAccum + change * (int)vertices.size();
        hasLast = false;

        return {"worldedit.shift.shifted", true};
    }

    bool ConvexRegion::contains(const BlockPos& pos) {
        if (triangles.empty()) {
            return false;
        }

        if (!pos.containedWithin(boundingBox.min, boundingBox.max)) {
            return false;
        }

        return containsRaw(pos.toVec3());
    }

    void ConvexRegion::forEachLine(const std::function<void(const BlockPos&, const BlockPos&)>& todo) {
        if (poss.size() > 1) {
            for (int i = 0; i < poss.size() - 1; ++i) {
                todo(poss[i], poss[i + 1]);
            }
        }
    }

    uint64_t ConvexRegion::size() const {
        double volume = 0;
        for (auto triangle : triangles) {
            volume += triangle.getVertex(0).cross(triangle.getVertex(1)).dot(triangle.getVertex(2));
        }
        return (uint64_t)std::abs(volume / 6.0);
    };

    void ConvexRegion::renderRegion() {
        if (selecting && dimensionID >= 0 && rendertick <= 0) {
            rendertick = 40;
            auto size = vertices.size();
            for (auto& vertice : vertices) {
                globalPT().drawCuboid(vertice, dimensionID, mce::ColorPalette::GREEN);
            }
            for (auto& pos : poss) {
                if (vertices.find(pos) == vertices.end()) {
                    globalPT().drawCuboid(pos, dimensionID, mce::ColorPalette::WHITE);
                }
            }
            for (auto& edge : edges) {
                globalPT().drawOrientedLine(edge.start + 0.5f, edge.end + 0.5f, dimensionID,
                                            ParticleCUI::PointSize::PX4, 1, 64, mce::ColorPalette::YELLOW);
            }
        }
        rendertick--;
    };

    bool Edge::operator==(const Edge& other) const {
        return (start == other.start && end == other.end) || (end == other.start && start == other.end);
    }

    Triangle Edge::createTriangle(const Vec3& vertex) {
        return Triangle(start, end, vertex);
    }

    Triangle::Triangle(const Vec3& v0, const Vec3& v1, const Vec3& v2) {
        vertices[0] = v0;
        vertices[1] = v1;
        vertices[2] = v2;
        normal = ((v1 - v0).cross(v2 - v0)).normalize();
        maxDotProduct = std::max(std::max(normal.dot(v0), normal.dot(v1)), normal.dot(v2));
    }
    bool Triangle::operator==(const Triangle& v) const {
        return (v.maxDotProduct == maxDotProduct) && (v.normal == normal) && (v.vertices[0] == vertices[0]) &&
               (v.vertices[1] == vertices[1]) && (v.vertices[2] == vertices[2]);
    }
    Edge Triangle::getEdge(const int& index) {
        if (index == 2)
            return Edge(vertices[index], vertices[0]);
        return Edge(vertices[index], vertices[index + 1]);
    }
}  // namespace worldedit