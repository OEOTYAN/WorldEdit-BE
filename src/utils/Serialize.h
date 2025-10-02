#pragma once

#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>

#include <mc/deps/core/math/Color.h>
#include <mc/deps/core/utility/AutomaticID.h>

namespace mce {
template <class J>
inline ll::Expected<J> serialize(Color const& c) {
    return std::format("#{:06x}", int(c.r * 255.0f) * (1u << 16) + int(c.g * 255.0f) * (1u << 8) + int(c.b * 255.0f))
         + (c.a == 1 ? "" : std::format("{:02x}", static_cast<int>(c.a * 255.0f)));
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
