#include "GeoContainer.h"
#include "worldedit/WorldEdit.h"

namespace we {
GeoContainer::GeoContainer() noexcept : geo({0}) {}

GeoContainer::GeoContainer(bsci::GeometryGroup::GeoId id) noexcept : geo(id) {}

GeoContainer::GeoContainer(GeoContainer&& other) noexcept : GeoContainer(other.geo) {
    other.geo = {0};
}
GeoContainer& GeoContainer::operator=(GeoContainer&& other) noexcept {
    if (this == std::addressof(other)) {
        return *this;
    }
    WorldEdit::getInstance().getGeo().remove(geo);
    geo       = other.geo;
    other.geo = {0};
    return *this;
}
GeoContainer::~GeoContainer() { WorldEdit::getInstance().getGeo().remove(geo); }
} // namespace we
