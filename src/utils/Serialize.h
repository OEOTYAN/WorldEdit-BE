#pragma once

#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>

#include <mc/deps/core/mce/Color.h>
#include <mc/world/AutomaticID.h>

namespace mce {
template <class J>
inline ll::Expected<J> serialize(Color const& c) {
    return std::format("#{:06x}", (uint)c.toARGB() % (1u << 24))
         + (c.a == 1 ? "" : std::format("{:02x}", (uint)c.toARGB() / (1u << 24)));
}
template <class T, class J>
    requires(std::same_as<Color, T>)
inline ll::Expected<> deserialize(Color& v, J const& j) {
    if (j.is_string()) {
        v = mce::Color{(std::string_view)j};
    }
    return {};
}
} // namespace mce

template <class J>
inline ll::Expected<J> serialize(DimensionType const& c) {
    return c.id;
}
template <class T, class J>
    requires(std::same_as<DimensionType, T>)
inline ll::Expected<> deserialize(DimensionType& v, J const& j) {
    v.id = j;
    return {};
}
