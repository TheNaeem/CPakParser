#pragma once

// TODO: https://github.com/greg7mdp/gtl

#include "parallel_hashmap/phmap.h"

import CPakParser.Serialization.FArchive;

using phmap::flat_hash_map;
using phmap::flat_hash_set;
using phmap::parallel_flat_hash_map;

template <typename K, typename V>
using TMap = flat_hash_map<K, V>; // TODO: make this a module

template<class T>
FArchive& operator<<(FArchive& Ar, flat_hash_set<T>& InSet)
{
	int32_t NewNumElements = 0;
	Ar << NewNumElements;

	if (!NewNumElements) return Ar;

	InSet.clear();
	InSet.reserve(NewNumElements);

	for (size_t i = 0; i < NewNumElements; i++)
	{
		T Element;
		Ar << Element;
		InSet.insert(Element);
	}

	return Ar;
}

template<class Key, class Value>
FArchive& operator<<(FArchive& Ar, TMap<Key, Value>& InMap)
{
	auto Pairs = flat_hash_set<std::pair<Key, Value>>();
	
	int32_t NewNumElements = 0;
	Ar << NewNumElements;

	if (!NewNumElements) return Ar;

	InMap.reserve(NewNumElements);

	for (size_t i = 0; i < NewNumElements; i++)
	{
		auto Pair = std::pair<Key, Value>();

		Ar << Pair;

		InMap.insert_or_assign(Pair.first, Pair.second);
	}

	return Ar;
}