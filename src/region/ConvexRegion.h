#pragma once

#include "Region.h"
#include "utils/Triangle.h"

namespace we {
class ConvexRegion : public Region {
    bool addVertex(BlockPos const& vertex);

    bool addVertexWithIndex(BlockPos const& pos);

    bool containsRaw(BlockPos const& pt) const;

    void updateEdges();

    ll::SmallDenseSet<BlockPos>      vertices;
    std::vector<Triangle>            triangles;
    ll::SmallDenseSet<WithGeo<Edge>> edges;
    ll::SmallDenseSet<BlockPos>      vertexBacklog;

    ll::math::longlong3 centerAccum;

    mutable std::optional<Triangle> lastTriangle;

    std::vector<WithGeo<BlockPos>> indexedVertices;

public:
    ConvexRegion(DimensionType, BoundingBox const&);

    ll::Expected<> serialize(CompoundTag&) const override;

    ll::Expected<> deserialize(CompoundTag const&) override;

    void updateBoundingBox() override;

    RegionType getType() const override { return RegionType::Convex; }

    size_t size() const override;

    bool shift(BlockPos const&) override;

    Vec3 getCenter() const override {
        return Vec3{centerAccum} * (1.0f / vertices.size()) + 0.5;
    };

    std::vector<std::string> getInfo() const override;

    ll::coro::Generator<std::pair<BlockPos, BlockPos>> forEachLine() const override;

    bool setMainPos(BlockPos const&) override;

    bool setOffPos(BlockPos const&) override;

    bool contains(BlockPos const&) const override;

    bool removePoint(std::optional<BlockPos> const&) override;
};
} // namespace we
