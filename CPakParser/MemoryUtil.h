#pragma once

struct MemoryUtil
{
	template <typename T>
	static __forceinline T Read(const void* Ptr) 
	{
		return *reinterpret_cast<const T*>(Ptr);
	}

	template <typename T>
	static __forceinline void Write(void* Ptr, const T& Value)
	{
		*reinterpret_cast<T*>(Ptr) = Value;
	}
};