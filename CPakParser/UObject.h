#pragma once

#include "CoreTypes.h"

template <typename ObjectType>
class TObjectPtr
{
	//static_assert(std::is_base_of<UObject, ObjectType>::value, "Type passed into UObjectPtr must be a UObject type");

	TSharedPtr<ObjectType> Val;

public:

	TObjectPtr() : Val(nullptr)
	{
	}

	TObjectPtr operator=(TSharedPtr<ObjectType> Other)
	{
		Val = Other;
		return *this;
	}

	TObjectPtr(TSharedPtr<ObjectType> InObject) : Val(InObject)
	{
	}

	ObjectType* operator->()
	{
		if (!Val->IsLoaded())
		{
			Val->Load();
		}

		return Val.get();
	}

	operator bool() const
	{
		return Val.operator bool();
	}

	ObjectType* Get()
	{
		return Val.get();
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
};

typedef TObjectPtr<class UObject> UObjectPtr;
typedef TObjectPtr<class UClass> UClassPtr;
typedef TObjectPtr<class UStruct> UStructPtr;

class UObject
{
public:

	friend class UZenPackage;

protected:

	UClassPtr Class;
	UObjectPtr Outer;
	std::string Name;
	EObjectFlags Flags;

public:

	std::string GetName();
	UClassPtr GetClass();
	UObjectPtr GetOuter();

	void SetName(std::string& Val);
	void SetClass(UClassPtr Val);
	void SetOuter(UObjectPtr Val);
	
	bool IsLoaded();
	void Copy(UObjectPtr Other);
	void SerializeScriptProperties(TSharedPtr<class FExportReader> Ar);
	virtual void Serialize(TSharedPtr<class FExportReader> Ar);

	__forceinline void SetFlags(EObjectFlags NewFlags)
	{
		Flags = static_cast<EObjectFlags>(Flags & NewFlags);
	}

	__forceinline void SetFlagsTo(EObjectFlags NewFlags)
	{
		Flags = NewFlags;
	}

	__forceinline EObjectFlags GetFlags()
	{
		return Flags;
	}

	__forceinline void ClearFlags(EObjectFlags NewFlags)
	{
		Flags = static_cast<EObjectFlags>(Flags & ~NewFlags);
	}

	__forceinline bool HasAnyFlags(EObjectFlags FlagsToCheck) const
	{
		return (Flags & FlagsToCheck) != 0;
	}

	virtual void Load() { }
};