#include "TextProperty.h"

#include "Serialization/Archives.h"
#include "Misc/Localization/Text.h"

TUniquePtr<IPropValue> FTextProperty::Serialize(FArchive& Ar)
{
	auto Ret = std::make_unique<Value>();
	Ar << Ret->Text;

	return Ret;
}