module;

#include "Core/Defines.h"

export module CPakParser.Reflection.DelegateProperty;

export import CPakParser.Reflection.FProperty;
import CPakParser.Delegates.MulticastScriptDelegate;
import CPakParser.Serialization.FArchive;

export struct FDelegateProperty : public FProperty
{
	struct Value : public IPropValue
	{
		FScriptDelegate Delegate;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::DelegateProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			if (Type == EPropertyType::DelegateProperty)
			{
				memcpy(OutBuffer, &Delegate, sizeof(Delegate));
			}
			else if (Type == EPropertyType::StrProperty)
			{
				*((std::string*)OutBuffer) = Delegate.GetFunctionName();
			}
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{
		auto Ret = std::make_unique<Value>();
		Ar << Ret->Delegate;

		return std::move(Ret);
	}
};

export struct FMulticastDelegateProperty : public FProperty
{
	struct Value : public IPropValue
	{
		FMulticastScriptDelegate Delegate;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::MulticastDelegateProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			if (Type == EPropertyType::MulticastDelegateProperty)
			{
				*((FMulticastScriptDelegate*)OutBuffer) = Delegate;
			}
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{
		auto Ret = std::make_unique<Value>();
		Ar << Ret->Delegate;

		return std::move(Ret);
	}
};