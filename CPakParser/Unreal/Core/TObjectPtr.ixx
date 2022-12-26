module;

#include "Core/Defines.h"

export module CPakParser.Core.TObjectPtr;

export template <typename ObjectType>
class TObjectPtr
{
	//static_assert(std::is_base_of<UObject, ObjectType>::value, "Type passed into UObjectPtr must be a UObject type");

	TSharedPtr<ObjectType> Val;

public:

	TObjectPtr() : Val(nullptr)
	{
	}

	TObjectPtr(TSharedPtr<ObjectType> InObject) : Val(InObject)
	{
	}

	TObjectPtr(TSharedPtr<ObjectType>& InObject) : Val(InObject)
	{
	}

	TObjectPtr(std::nullptr_t Null) : Val(nullptr)
	{
	}

	__forceinline TObjectPtr operator=(TSharedPtr<ObjectType> Other)
	{
		Val = Other;
		return *this;
	}

	template <typename T>
	__forceinline bool operator==(TObjectPtr<T>& Other)
	{
		return Val == Other.Val;
	}

	template <typename T>
	__forceinline bool operator!=(TObjectPtr<T>& Other)
	{
		return !operator==(Other);
	}

	__forceinline ObjectType* operator->()
	{
		return Val.get();
	}

	__forceinline operator bool() const
	{
		return Val.operator bool();
	}

	__forceinline ObjectType* Get()
	{
		return Val.get();
	}

	__forceinline ObjectType& operator*()
	{
		return *Get();
	}

	template <typename T>
	__forceinline bool IsA()
	{
		return std::dynamic_pointer_cast<T>(Val);
	}

	template <typename T>
	__forceinline TSharedPtr<T> As()
	{
		return std::dynamic_pointer_cast<T>(Val);
	}

	template <typename T>
	__forceinline operator TObjectPtr<T>() const
	{
		return TObjectPtr<T>(std::dynamic_pointer_cast<T>(Val));
	}

	template <typename T>
	__forceinline operator TObjectPtr<T>()
	{
		return TObjectPtr<T>(std::dynamic_pointer_cast<T>(Val));
	}
};