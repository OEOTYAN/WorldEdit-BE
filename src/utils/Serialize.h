#pragma once

#include <ll/api/reflection/Deserialization.h>
#include <ll/api/reflection/Serialization.h>

namespace we {
template <class T, class J>
inline void doSerialize(T const& t, J& j) {
    j = ::ll::reflection::serialize<J>(t);
}
template <class T, class J>
inline T doDeserialize(J const& j) {
    T t;
    ::ll::reflection::deserialize<J>(t, j);
    return t;
}
} // namespace we
