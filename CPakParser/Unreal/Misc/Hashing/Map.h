#pragma once

#include "parallel_hashmap/phmap.h"

template <class K, class V>
using TMap = phmap::flat_hash_map<K, V>;