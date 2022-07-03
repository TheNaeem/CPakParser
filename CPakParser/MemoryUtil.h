#pragma once

#include <vector>

namespace MemoryUtil
{
	__pragma(pack(push, 1))
		template <typename U>
	struct TUnaligned
	{
		U Value;
	};
	__pragma(pack(pop))

	template <typename T>
	static __forceinline T Read(const void* Ptr) 
	{
		return *reinterpret_cast<const T*>(Ptr);
	}

	template <typename T>
	static __forceinline T ReadUnaligned(const void* Ptr)
	{
		return reinterpret_cast<const TUnaligned<T>*>(Ptr)->Value;
	}

	template <typename T>
	static __forceinline void WriteUnaligned(void* Ptr, const T& InValue)
	{
		reinterpret_cast<TUnaligned<T>*>(Ptr)->Value = InValue;
	}

	template <typename T>
	static __forceinline void Write(void* Ptr, const T& Value)
	{
		*reinterpret_cast<T*>(Ptr) = Value;
	}

	template <typename T>
	static __forceinline constexpr T Align(T Val, uint64_t Alignment)
	{
		return (T)(((uint64_t)Val + Alignment - 1) & ~(Alignment - 1));
	}

	template <typename T>
	static __forceinline std::vector<T> ReadArray(T*& Src, int Count)
	{
		auto Ret = std::vector<T>(Src, Src + Count);
		Src += Count;
		return Ret;
	}
};