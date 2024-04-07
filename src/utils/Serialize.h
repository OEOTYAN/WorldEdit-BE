#pragma once

#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>

#include <mc/deps/core/mce/Color.h>
#include <mc/world/AutomaticID.h>

namespace mce {
template <class J>
inline J serialize(Color const& c) {
    return std::format("#{:06x}", (uint)c.toARGB() % (1u << 24))
         + (c.a == 1 ? "" : std::format("{:02x}", (uint)c.toARGB() / (1u << 24)));
}
template <class J>
inline void deserialize(Color& v, J const& j) {
    if (j.is_string()) {
        v = mce::Color{(std::string_view)j};
    }
}
} // namespace mce

template <class J>
inline J serialize(DimensionType const& c) {
    return c.id;
}
template <class J>
inline void deserialize(DimensionType& v, J const& j) {
    if (j.is_number()) {
        v.id = j;
    }
}

namespace we {
template <class T, class J>
inline void doSerialize(T const& t, J& j) {
    using namespace ll::reflection;
    j = serialize<J>(t);
}
template <class T, class J>
inline T doDeserialize(J const& j) {
    using namespace ll::reflection;
    T t;
    deserialize<J>(t, j);
    return t;
}
} // namespace we
