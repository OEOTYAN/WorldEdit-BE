#pragma once

#include <ll/api/base/Concepts.h>
#include <parallel_hashmap/phmap_utils.h>

namespace std {
template <ll::concepts::Require<phmap::has_hash_value> T>
struct hash<T> {
    inline size_t operator()(T const& val) const { return hash_value(val); }
};
}