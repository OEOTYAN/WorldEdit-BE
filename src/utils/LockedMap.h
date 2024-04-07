#pragma once

#include <parallel_hashmap/phmap.h>

#include <shared_mutex>

namespace we {
template <class K, class V, size_t N = 4, class M = std::shared_mutex>
using pfh_map = phmap::parallel_flat_hash_map<
    K,
    V,
    phmap::priv::hash_default_hash<K>,
    phmap::priv::hash_default_eq<K>,
    phmap::priv::Allocator<phmap::priv::Pair<const K, V>>,
    N,
    M>;
}
