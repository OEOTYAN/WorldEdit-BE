#include "ConvexRegion.h"
#include "worldedit/WorldEdit.h"

namespace we {
void ConvexRegion::updateBoundingBox() {
    boundingBox.min = *vertices.begin();
    boundingBox.max = *vertices.begin();
    for (auto& v : vertices) {
        boundingBox.min.x = std::min(boundingBox.min.x, v.x);
        boundingBox.min.y = std::min(boundingBox.min.y, v.y);
        boundingBox.min.z = std::min(boundingBox.min.z, v.z);
        boundingBox.max.x = std::max(boundingBox.max.x, v.x);
        boundingBox.max.y = std::max(boundingBox.max.y, v.y);
        boundingBox.max.z = std::max(boundingBox.max.z, v.z);
    }
    for (auto& v : vertexBacklog) {
        boundingBox.min.x = std::min(boundingBox.min.x, v.x);
        boundingBox.min.y = std::min(boundingBox.min.y, v.y);
        boundingBox.min.z = std::min(boundingBox.min.z, v.z);
        boundingBox.max.x = std::max(boundingBox.max.x, v.x);
        boundingBox.max.y = std::max(boundingBox.max.y, v.y);
        boundingBox.max.z = std::max(boundingBox.max.z, v.z);
    }
}

void ConvexRegion::updateEdges() {
    auto temp{std::move(edges)};
    for (auto& triangle : triangles) {
        for (int i = 0; i < 3; ++i) {
            Edge edge = triangle.getEdge(i);
            if (auto iter = temp.find(edge); iter != temp.end()) {
                edges.emplace(edge, std::move(iter->geo));
            } else {
                edges.emplace(
                    edge,
                    WorldEdit::getInstance().getGeo().line(
                        getDim(),
                        Vec3{edge.start} + 0.5,
                        Vec3{edge.end} + 0.5,
                        WorldEdit::getInstance().getConfig().colors.region_line_color
                    )
                );
            }
        }
    }
}

ConvexRegion::ConvexRegion(DimensionType d, BoundingBox const& b) : Region(d, b) {}

bool ConvexRegion::containsRaw(BlockPos const& pt) const {
    if (hasLast && lastTriangle.above(pt)) {
        return false;
    }
    for (auto& triangle : triangles) {
        if (lastTriangle == triangle) {
            continue;
        }
        if (triangle.above(pt)) {
            hasLast      = true;
            lastTriangle = triangle;
            return false;
        }
    }
    return true;
}

bool ConvexRegion::addVertex(BlockPos const& vertex) {
    hasLast = false;
    if (vertices.contains(vertex)) {
        return false;
    }
    if (vertices.size() == 3) {
        if (vertexBacklog.contains(vertex)) {
            return false;
        }
        if (containsRaw(vertex)) {
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
        triangles.emplace_back(
            indexedVertices[0].data,
            indexedVertices[1].data,
            indexedVertices[2].data
        );
        triangles.emplace_back(
            indexedVertices[0].data,
            indexedVertices[2].data,
            indexedVertices[1].data
        );
        updateEdges();
        return true;
    };
    default:
        break;
    }
    phmap::flat_hash_set<Edge> borderEdges;
    for (auto iter = triangles.begin(); iter != triangles.end();) {
        if ((*iter).above(vertex)) {
            for (int i = 0; i < 3; ++i) {
                Edge edge = (*iter).getEdge(i);
                if (!borderEdges.contains(edge)) {
                    borderEdges.insert(edge);
                } else {
                    borderEdges.erase(edge);
                }
            }
            iter = triangles.erase(iter);
        } else {
            ++iter;
        }
    }
    for (auto& edge : borderEdges) {
        triangles.emplace_back(edge.start, edge.end, vertex);
    }
    if (!vertexBacklog.empty()) {
        vertices.erase(vertex);
        phmap::flat_hash_set<BlockPos> vertexBacklog2(std::move(vertexBacklog));
        for (auto& vertex2 : vertexBacklog2) {
            addVertex(vertex2);
        }
        vertices.insert(vertex);
    }
    updateEdges();
    return true;
}

bool ConvexRegion::addVertexWithIndex(BlockPos const& pos) {
    bool res = addVertex(pos);
    if (res) {
        indexedVertices.emplace_back(
            pos,
            WorldEdit::getInstance().getGeo().box(
                getDim(),
                pos,
                WorldEdit::getInstance().getConfig().colors.region_point_color
            )
        );
    }
    return res;
}

bool ConvexRegion::setMainPos(BlockPos const& pos) {
    hasLast = false;
    vertices.clear();
    triangles.clear();
    vertexBacklog.clear();
    edges.clear();
    indexedVertices.clear();
    centerAccum = 0;
    return addVertexWithIndex(pos);
}

bool ConvexRegion::setVicePos(BlockPos const& pos) { return addVertexWithIndex(pos); }

bool ConvexRegion::shift(BlockPos const& change) {
    boundingBox.min = boundingBox.min + change;
    boundingBox.max = boundingBox.max + change;
    auto tmpVertices{std::move(vertices)};
    for (auto& vertice : tmpVertices) {
        vertices.insert(vertice + change);
    }
    auto& geo = WorldEdit::getInstance().getGeo();
    for (auto& pos : indexedVertices) {
        pos.data += change;
        geo.shift(pos.geo.geo, change);
    }
    auto tmpVertexBacklog{std::move(vertexBacklog)};
    for (auto& vertex : tmpVertexBacklog) {
        vertexBacklog.emplace(vertex + change);
    }
    for (auto& triangle : triangles) {
        triangle.vertices[0]   += change;
        triangle.vertices[1]   += change;
        triangle.vertices[2]   += change;
        triangle.maxDotProduct  = triangle.evalMaxDotProduct();
    }
    auto tmpEdges{std::move(edges)};
    for (auto& e : tmpEdges) {
        geo.shift(e.geo.geo, change);
        edges.emplace(
            Edge({e.data.start + change, e.data.end + change}),
            std::move(e.geo)
        );
    }
    centerAccum = centerAccum + change * (int)vertices.size();
    hasLast     = false;

    return true;
}

bool ConvexRegion::contains(BlockPos const& pos) const {
    if (triangles.empty()) {
        return false;
    }

    if (!boundingBox.contains(pos)) {
        return false;
    }

    return containsRaw(pos);
}

void ConvexRegion::forEachLine(
    std::function<void(BlockPos const&, BlockPos const&)>&& todo
) const {
    if (indexedVertices.size() > 1) {
        for (int i = 0; i < indexedVertices.size() - 1; ++i) {
            todo(indexedVertices[i].data, indexedVertices[i + 1].data);
        }
    }
}

uint64_t ConvexRegion::size() const {
    double volume = 0;
    for (auto triangle : triangles) {
        volume +=
            triangle.getVertex(0).cross(triangle.getVertex(1)).dot(triangle.getVertex(2));
    }
    return (uint64_t)std::abs(volume / 6.0);
};
} // namespace we
