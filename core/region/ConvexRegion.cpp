//
// Created by OEOTYAN on 2021/2/8.
//

#include "ConvexRegion.h"
namespace worldedit {
    void ConvexRegion::updateBoundingBox() {
        rendertick = 0;
        boundingBox.bpos1 = *vertices.begin();
        boundingBox.bpos2 = *vertices.begin();
        for_each(vertices.begin(), vertices.end(), [&](BlockPos value) {
            boundingBox.bpos1.x = std::min(boundingBox.bpos1.x, value.x);
            boundingBox.bpos1.y = std::min(boundingBox.bpos1.y, value.y);
            boundingBox.bpos1.z = std::min(boundingBox.bpos1.z, value.z);
            boundingBox.bpos2.x = std::max(boundingBox.bpos2.x, value.x);
            boundingBox.bpos2.y = std::max(boundingBox.bpos2.y, value.y);
            boundingBox.bpos2.z = std::max(boundingBox.bpos2.z, value.z);
        });
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

    ConvexRegion::ConvexRegion(const BoundingBox& region, const int& dim)
        : Region(region, dim) {
        vertices.clear();
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
                triangles.emplace_back(
                    Triangle(v[0].toVec3(), v[1].toVec3(), v[2].toVec3()));
                triangles.emplace_back(
                    Triangle(v[0].toVec3(), v[2].toVec3(), v[1].toVec3()));
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
            std::unordered_set<BlockPos, _hash> vertexBacklog2(vertexBacklog);
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
        centerAccum = BlockPos(0, 0, 0);
        return addVertex(pos);
    }

    bool ConvexRegion::setVicePos(const BlockPos& pos, const int& dim) {
        if (!selecting || dim != dimensionID) {
            return false;
        }
        return addVertex(pos);
    }

    bool ConvexRegion::shift(const BlockPos& change, Player* player) {
        boundingBox.bpos1 = boundingBox.bpos1 + change;
        boundingBox.bpos2 = boundingBox.bpos2 + change;
        auto tmpVertices = new std::unordered_set<BlockPos, _hash>(vertices);
        vertices.clear();
        for (auto vertice : *tmpVertices) {
            vertices.insert(vertice + change);
        }
        delete tmpVertices;
        auto tmpVertexBacklog =
            new std::unordered_set<BlockPos, _hash>(vertexBacklog);
        vertexBacklog.clear();
        for (auto vertex : *tmpVertexBacklog) {
            vertexBacklog.insert(vertex + change);
        }
        delete tmpVertexBacklog;
        for (auto triangle : triangles) {
            Vec3 v0 = triangle.getVertex(0) + change.toVec3();
            Vec3 v1 = triangle.getVertex(1) + change.toVec3();
            Vec3 v2 = triangle.getVertex(2) + change.toVec3();
            triangle = Triangle(v0, v1, v2);
        }
        auto tmpEdges = new std::unordered_set<Edge, _hash>(edges);
        edges.clear();
        for (auto edge : *tmpEdges) {
            edges.insert(
                Edge(edge.start + change.toVec3(), edge.end + change.toVec3()));
        }
        delete tmpEdges;
        centerAccum = centerAccum + change * (int)vertices.size();
        hasLast = false;
        player->sendFormattedText("§aThis region has been shifted");
        return true;
    }

    bool ConvexRegion::contains(const BlockPos& pos) {
        if (triangles.empty()) {
            return false;
        }

        if (!pos.containedWithin(boundingBox.bpos1, boundingBox.bpos2)) {
            return false;
        }

        return containsRaw(pos.toVec3());
    }

    int ConvexRegion::size() const {
        double volume = 0;
        for (auto triangle : triangles) {
            volume += triangle.getVertex(0)
                          .cross(triangle.getVertex(1))
                          .dot(triangle.getVertex(2));
        }
        return (int)std::abs(volume / 6.0);
    };

    void ConvexRegion::renderRegion() {
        if (selecting && dimensionID >= 0 && rendertick <= 0) {
            rendertick = 40;
            auto size = vertices.size();
            for (auto vertice : vertices)
                worldedit::spawnCuboidParticle(
                    {vertice.toVec3(), vertice.toVec3() + Vec3::ONE},
                    GRAPHIC_COLOR::GREEN, dimensionID);
            for (auto edge : edges)
                drawOrientedLine(edge.start, edge.end, dimensionID);
        }
        rendertick--;
    };

    bool Edge::operator==(const Edge& other) const {
        return (start == other.start && end == other.end) ||
               (end == other.start && start == other.end);
    }

    Triangle Edge::createTriangle(const Vec3& vertex) {
        return Triangle(start, end, vertex);
    }

    Triangle::Triangle(const Vec3& v0, const Vec3& v1, const Vec3& v2) {
        vertices[0] = v0;
        vertices[1] = v1;
        vertices[2] = v2;
        normal = ((v1 - v0).cross(v2 - v0)).normalize();
        maxDotProduct =
            std::max(std::max(normal.dot(v0), normal.dot(v1)), normal.dot(v2));
    }
    bool Triangle::operator==(const Triangle& v) const {
        return (v.maxDotProduct == maxDotProduct) && (v.normal == normal) &&
               (v.vertices == vertices);
    }
    Edge Triangle::getEdge(const int& index) {
        if (index == 2)
            return Edge(vertices[index], vertices[0]);
        return Edge(vertices[index], vertices[index + 1]);
    }
}  // namespace worldedit