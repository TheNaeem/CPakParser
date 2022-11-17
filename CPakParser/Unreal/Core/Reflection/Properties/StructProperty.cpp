#include "Core/Defines.h"
#include "Misc/Hashing/Map.h"
#include <functional>

import CPakParser.Reflection.PropertyValue;
import CPakParser.Reflection.StructProperty;
import CPakParser.Misc.FGuid;
import CPakParser.Paths.SoftObjectPath;
import CPakParser.Math.FBox;
import CPakParser.Structs.DateTime;
import CPakParser.Structs.GameplayTagContainer;
import CPakParser.Serialization.FArchive;

template <typename StructType>
static __forceinline TUniquePtr<IPropValue> SerializeNativeStruct(FArchive& Ar)
{
	auto Ret = std::make_unique<FStructProperty::NativeValue<StructType>>();
	Ar << Ret->Value;

	return std::move(Ret);
}

static TMap<std::string, std::function<TUniquePtr<IPropValue>(FArchive&)>> NativeStructs =
{
	{ "Guid", SerializeNativeStruct<FGuid> },
	{ "Vector", SerializeNativeStruct<FVector> },
	{ "Vector2D", SerializeNativeStruct<FVector2D> },
	{ "GameplayTagContainer", SerializeNativeStruct<FGameplayTagContainer> },
	{ "SoftObjectPath", SerializeNativeStruct<FSoftObjectPath> },
	{ "DateTime", SerializeNativeStruct<FDateTime> },
	{ "Box", SerializeNativeStruct<FBox> }
};

TUniquePtr<IPropValue> FStructProperty::Serialize(FArchive& Ar)
{
	auto StructName = Struct->GetName();
	if (NativeStructs.contains(StructName))
	{
		return std::move(NativeStructs[StructName](Ar));
	}

	auto Ret = std::make_unique<Value>();

	Ret->StructObject = std::make_shared<UObject>();
	Ret->StructObject->SetClass(Struct);

	Struct->SerializeScriptProperties(Ar, Ret->StructObject);

	return std::move(Ret);
}