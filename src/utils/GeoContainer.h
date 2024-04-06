#pragma once

#include "worldedit/WorldEdit.h"

namespace we {
struct GeoContainer {
    bsci::GeometryGroup::GeoId geo;

    GeoContainer(GeoContainer const&) noexcept            = delete;
    GeoContainer& operator=(GeoContainer const&) noexcept = delete;

    GeoContainer() noexcept : geo({0}) {}

    GeoContainer(bsci::GeometryGroup::GeoId id) noexcept : geo(id) {}

    GeoContainer(GeoContainer&& other) noexcept : GeoContainer(other.geo) {
        other.geo = {0};
    }
    GeoContainer& operator=(GeoContainer&& other) noexcept {
        if (this == std::addressof(other)) {
            return *this;
        }
        WorldEdit::getInstance().getGeo().remove(geo);
        geo       = other.geo;
        other.geo = {0};
        return *this;
    }
    ~GeoContainer() { WorldEdit::getInstance().getGeo().remove(geo); }
};
template <class T>
struct WithGeo {
    T data;
    GeoContainer mutable geo;
    WithGeo() = default;
    WithGeo(T const& t) : data(t) {}
    WithGeo(T const& t, GeoContainer&& geo) : data(t), geo(std::move(geo)) {}

    WithGeo(WithGeo const&) noexcept            = delete;
    WithGeo& operator=(WithGeo const&) noexcept = delete;
    WithGeo(WithGeo&&) noexcept                 = default;
    WithGeo& operator=(WithGeo&&) noexcept      = default;

    constexpr bool operator==(WithGeo const& other) const { return data == other.data; }
};
template <class T>
inline size_t hash_value(WithGeo<T> const& d) {
    return std::hash<T>()(d.data);
}
} // namespace we
