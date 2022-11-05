#pragma once

#include "../Property.h"
#include "../PropertyValue.h"
#include "Misc/Paths/SoftObjectPath.h"

class FSoftObjectProperty : public FProperty
{
public:

	class Value : public IPropValue
	{
	public:

		FSoftObjectPath Path;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::SoftObjectProperty or Type == EPropertyType::ObjectProperty;
		}

		void PlaceValue(EPropertyType Type, void* OutBuffer)
		{
			if (Type == EPropertyType::SoftObjectProperty)
			{
				memcpy(OutBuffer, &Path, sizeof(Path));
			}
			else if (Type == EPropertyType::ObjectProperty)
			{
				//TODO: 
			}
		}
	};

	TUniquePtr<IPropValue> Serialize(FSharedAr Archive);
};