#include "StructProperty.h"

#include "Serialization/Archives.h"
#include "Misc/Guid.h"
#include "Structs/Math/Vector.h"

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
	{ "Vector", SerializeNativeStruct<FVector> }
};

TUniquePtr<IPropValue> FStructProperty::Serialize(FSharedAr Ar)
{
	auto StructName = Struct->GetName();
	if (NativeStructs.contains(StructName))
	{
		return NativeStructs[StructName](*Ar);
	}

	auto Ret = std::make_unique<Value>();

	Ret->StructObject = std::make_shared<UObject>();
	Ret->StructObject->SetClass(Struct);
	Ret->StructObject->SetName(this->Name);

	Struct->SerializeScriptProperties(Ar, Ret->StructObject);

	return Ret;
}