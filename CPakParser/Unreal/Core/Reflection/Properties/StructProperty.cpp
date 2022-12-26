#include "Core/Defines.h"
#include "Misc/Hashing/Map.h"
#include <functional>

import CPakParser.Logging;
import CPakParser.Reflection.PropertyValue;
import CPakParser.Reflection.StructProperty;
import CPakParser.Misc.FGuid;
import CPakParser.Paths.SoftObjectPath;
import CPakParser.Math.FBox;
import CPakParser.Structs.DateTime;
import CPakParser.Math.Color;
import CPakParser.Structs.GameplayTagContainer;
import CPakParser.Serialization.FArchive;
import CPakParser.Materials.Expression;
import CPakParser.Math.FrameNumber;
import CPakParser.AI.Navigation;
import CPakParser.Names.SmartName;
import CPakParser.Curves.RichCurve;
import CPakParser.Curves.SimpleCurve;
import CPakParser.Math.IntPoint;
import CPakParser.Math.PerPlatform;

template <typename StructType>
static __forceinline TUniquePtr<IPropValue> SerializeNativeStruct(FArchive& Ar)
{
	auto Ret = std::make_unique<FStructProperty::NativeValue<StructType>>();
	Ar << Ret->Value;

	return std::move(Ret);
}

static TMap<std::string, std::function<TUniquePtr<IPropValue>(FArchive&)>> NativeStructs =
{
	{ "Box", SerializeNativeStruct<FBox> },
	{ "Box2D", SerializeNativeStruct<FBox2D> },
	{ "Color", SerializeNativeStruct<FColor> },
	{ "ColorMaterialInput", SerializeNativeStruct<FColorMaterialInput> },
	{ "DateTime", SerializeNativeStruct<FDateTime> },
	{ "ExpressionInput", SerializeNativeStruct<FExpressionInput> },
	{ "FrameNumber", SerializeNativeStruct<FFrameNumber> },
	{ "GameplayTagContainer", SerializeNativeStruct<FGameplayTagContainer> },
	{ "Guid", SerializeNativeStruct<FGuid> },
	{ "IntPoint", SerializeNativeStruct<FIntPoint> },
	{ "LinearColor", SerializeNativeStruct<FLinearColor> },
	{ "NavAgentSelector", SerializeNativeStruct<FNavAgentSelector> },
	{ "NavAgentSelector", SerializeNativeStruct<FNavAgentSelector> },
	{ "RichCurveKey", SerializeNativeStruct<FRichCurveKey> },
	{ "PerPlatformFloat", SerializeNativeStruct<FPerPlatformFloat> },
	{ "SimpleCurveKey", SerializeNativeStruct<FSimpleCurveKey> },
	{ "SmartName", SerializeNativeStruct<FSmartName> },
	{ "SoftObjectPath", SerializeNativeStruct<FSoftObjectPath> },
	{ "Vector", SerializeNativeStruct<FVector> },
	{ "Vector2D", SerializeNativeStruct<FVector2D> },
	{ "Vector4", SerializeNativeStruct<FVector4> }
};

TUniquePtr<IPropValue> FStructProperty::Serialize(FArchive& Ar)
{
	auto StructName = Struct->GetName();

#if EXTENSIVE_LOGGING
	Log("Serializing struct property of type %s", StructName.c_str());
#endif

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