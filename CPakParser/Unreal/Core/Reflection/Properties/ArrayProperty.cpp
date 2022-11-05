#include "ArrayProperty.h"

#include "Serialization/Archives.h"

TUniquePtr<IPropValue> FArrayProperty::Serialize(FSharedAr Archive)
{
	auto Ret = std::make_unique<FArrayProperty::Value>();
	auto& Ar = *Archive;

	if (Ar.UseUnversionedPropertySerialization())
	{
		int32_t ArrayCount;
		Ar << ArrayCount;

		Ret->Array.resize(ArrayCount);

		for (size_t i = 0; i < ArrayCount; i++)
			Ret->Array[i] = ElementType->Serialize(Archive);

		return Ret;
	}

	return nullptr; // TODO: property tag crap
}