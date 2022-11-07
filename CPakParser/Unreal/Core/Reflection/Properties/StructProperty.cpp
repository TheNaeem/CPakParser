#include "StructProperty.h"

#include "Serialization/Archives.h"
#include "Misc/Guid.h"
#include "Structs/Math/Vector.h"
#include "Structs/Tags/GameplayTagContainer.h"

template <typename StructType>
static __forceinline TUniquePtr<IPropValue> SerializeNativeStruct(FArchive& Ar)
{
	auto Ret = std::make_unique<FStructProperty::NativeValue<StructType>>();
	Ar << Ret->Value;

	return Ret;
}

static TMap<std::string, std::function<TUniquePtr<IPropValue>(FArchive&)>> NativeStructs =
{
	{ "Guid", SerializeNativeStruct<FGuid> },
	{ "Vector", SerializeNativeStruct<FVector> },
	{ "GameplayTagContainer", SerializeNativeStruct<FGameplayTagContainer> }// TODO: add everything else
};

TUniquePtr<IPropValue> FStructProperty::Serialize(FArchive& Ar)
{
	auto StructName = Struct->GetName();
	if (NativeStructs.contains(StructName))
	{
		return NativeStructs[StructName](Ar);
	}

	auto Ret = std::make_unique<Value>();

	Ret->StructObject = std::make_shared<UObject>();
	Ret->StructObject->SetClass(Struct);

	Struct->SerializeScriptProperties(Ar, Ret->StructObject);

	return Ret;
}