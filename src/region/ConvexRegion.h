#pragma once

#include "Region.h"
#include "utils/Triangle.h"

namespace we {
class ConvexRegion : public Region {
    bool addVertex(BlockPos const& vertex);

    bool addVertexWithIndex(BlockPos const& pos);

    bool containsRaw(BlockPos const& pt) const;

    void updateEdges();

    phmap::flat_hash_set<BlockPos>      vertices;
    std::vector<Triangle>               triangles;
    phmap::flat_hash_set<WithGeo<Edge>> edges;
    phmap::flat_hash_set<BlockPos>      vertexBacklog;

    ll::math::longlong3 centerAccum;

    mutable std::optional<Triangle> lastTriangle;

    std::vector<WithGeo<BlockPos>> indexedVertices;

public:
    ConvexRegion(DimensionType, BoundingBox const&);

    ll::Expected<> serialize(CompoundTag&) const override;

    ll::Expected<> deserialize(CompoundTag const&) override;

    void updateBoundingBox() override;

    RegionType getType() const override { return RegionType::Convex; }

    uint64 size() const override;

    bool shift(BlockPos const&) override;

    Vec3 getCenter() const override {
        return Vec3{centerAccum} * (1.0f / vertices.size()) + 0.5;
    };

    void
    forEachLine(std::function<void(BlockPos const&, BlockPos const&)>&&) const override;

    bool setMainPos(BlockPos const&) override;

    bool setVicePos(BlockPos const&) override;

    bool contains(BlockPos const&) const override;

    bool removePoint(std::optional<BlockPos> const&) override;
};
} // namespace we
