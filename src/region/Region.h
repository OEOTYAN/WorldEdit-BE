#pragma once

#include "worldedit/WorldEdit.h"

#include "utils/GeoContainer.h"

namespace we {

class Region {
protected:
    BoundingBox         boundingBox;
    DimensionType const dim;

    Region(DimensionType, BoundingBox const&);

public:
    enum Type {
        Cuboid,
        Expand,
        Sphere,
        Poly,
        Convex,
        Cylinder,
        Loft,
    };
    static std::shared_ptr<Region>
    createRegion(Type, DimensionType, BoundingBox const&, bool update = true);

    static std::shared_ptr<Region> createRegion(CompoundTag const&);

    DimensionType getDim() const { return dim; }

    int getHeighest(Pos2d) const;

    virtual ~Region() = default;

    virtual void serialize(CompoundTag&) const;

    virtual void deserialize(CompoundTag const&);

    virtual bool needResetVice() const { return true; }

    virtual Type getType() const = 0;

    virtual BoundingBox getBoundBox() const { return boundingBox; }

    virtual void updateBoundingBox() = 0;

    virtual uint64_t size() const {
        return (uint64_t)(boundingBox.max.x - boundingBox.min.x + 1)
             * (boundingBox.max.y - boundingBox.min.y + 1)
             * (boundingBox.max.z - boundingBox.min.z + 1);
    }
    virtual bool expand(std::span<BlockPos>) { return false; }

    virtual bool contract(std::span<BlockPos>) { return false; }

    virtual bool shift(BlockPos const&) { return false; }

    virtual Vec3 getCenter() const { return AABB{boundingBox}.getCenter(); }

    virtual bool setMainPos(BlockPos const&) { return false; }

    virtual bool setVicePos(BlockPos const&) { return false; }

    virtual bool contains(BlockPos const& pos) const { return boundingBox.contains(pos); }

    virtual void forEachBlockInRegion(std::function<void(BlockPos const&)>&&) const;

    virtual void
    forEachBlockUVInRegion(std::function<void(BlockPos const&, double, double)>&&) const;

    virtual void
    forEachLine(std::function<void(BlockPos const&, BlockPos const&)>&&) const {}

    virtual bool removePoint(int, std::optional<BlockPos> const&) { return false; }
};
} // namespace we