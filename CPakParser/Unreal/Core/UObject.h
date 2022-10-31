#pragma once

#include "Defines.h"
#include "Reflection/PropertyValue.h"
#include <string>
#include <vector>

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

	TObjectPtr(TSharedPtr<ObjectType>& InObject) : Val(InObject)
	{
	}

	__forceinline ObjectType* operator->()
	{
		//if (!Val->IsLoaded())
		//{
		//	Val->Load(); // TODO:
		//}

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

	template <typename T>
	__forceinline operator TObjectPtr<T>() 
	{
		return TObjectPtr<T>(std::dynamic_pointer_cast<T>(Val));
	}
};

typedef TObjectPtr<class UObject> UObjectPtr;
typedef TObjectPtr<class UClass> UClassPtr;
typedef TObjectPtr<class UStruct> UStructPtr;

class UObject : public std::enable_shared_from_this<UObject>
{
public:

	UObject() = default;

	friend class UZenPackage;
	friend class FUnversionedSerializer;

	enum Flags
	{
		RF_NoFlags = 0x0,
		RF_Public = 0x1,
		RF_Standalone = 0x2,
		RF_MarkAsNative = 0x4,
		RF_Transactional = 0x8,
		RF_ClassDefaultObject = 0x10,
		RF_ArchetypeObject = 0x20,
		RF_Transient = 0x40,
		RF_MarkAsRootSet = 0x80,
		RF_TagGarbageTemp = 0x100,
		RF_NeedInitialization = 0x200,
		RF_NeedLoad = 0x400,
		RF_KeepForCooker = 0x800,
		RF_NeedPostLoad = 0x1000,
		RF_NeedPostLoadSubobjects = 0x2000,
		RF_NewerVersionExists = 0x4000,
		RF_BeginDestroyed = 0x8000,
		RF_FinishDestroyed = 0x10000,
		RF_BeingRegenerated = 0x20000,
		RF_DefaultSubObject = 0x40000,
		RF_WasLoaded = 0x80000,
		RF_TextExportTransient = 0x100000,
		RF_LoadCompleted = 0x200000,
		RF_InheritableComponentTemplate = 0x400000,
		RF_DuplicateTransient = 0x800000,
		RF_StrongRefOnFrame = 0x1000000,
		RF_NonPIEDuplicateTransient = 0x2000000,
		RF_Dynamic = 0x4000000,
		RF_WillBeLoaded = 0x8000000,
		RF_HasExternalPackage = 0x10000000,
	};

protected:

	UClassPtr Class;
	UObjectPtr Outer;
	std::string Name;
	Flags ObjectFlags;
	std::vector<std::pair<const std::string&, TUniquePtr<IPropValue>>> PropertyValues;

	template <typename T = UObject>
	__forceinline TObjectPtr<T> This()
	{
		return std::dynamic_pointer_cast<T>(shared_from_this());
	}

public:

	// Getters

	__forceinline std::string GetName()
	{
		return Name;
	}

	__forceinline UClassPtr GetClass()
	{
		return Class;
	}

	__forceinline UObjectPtr GetOuter()
	{
		return Outer;
	}

	// Setters

	__forceinline void SetName(std::string& Val)
	{
		Name = Val;
	}

	__forceinline void SetClass(UClassPtr Val)
	{
		Class = Val;
	}

	__forceinline void SetOuter(UObjectPtr Val)
	{
		Outer = Val;
	}

	// Flags

	__forceinline void SetFlags(Flags NewFlags)
	{
		ObjectFlags = static_cast<Flags>(ObjectFlags & NewFlags);
	}

	__forceinline void SetFlagsTo(Flags NewFlags)
	{
		ObjectFlags = NewFlags;
	}

	__forceinline Flags GetFlags()
	{
		return ObjectFlags;
	}

	__forceinline void ClearFlags(Flags NewFlags)
	{
		ObjectFlags = static_cast<Flags>(ObjectFlags & ~NewFlags);
	}

	__forceinline bool HasAnyFlags(Flags FlagsToCheck) const
	{
		return (ObjectFlags & FlagsToCheck) != 0;
	}

	__forceinline bool IsLoaded()
	{
		return ObjectFlags & RF_WasLoaded;
	}

	virtual void Load() { }

	virtual void Serialize(TSharedPtr<class FExportReader> Ar);
};