#include "Mappings.h"

#include "Core/Class.h"
#include "Serialization/Impl/FileReader.h"
#include "Serialization/Impl/MemoryReader.h"
#include "Misc/Compression/Oodle.h"
#include "Core/Reflection/Properties/PropertyTypes.h"
#include "Logger.h"

#define USMAP_FILE_MAGIC 0x30C4

static std::string InvalidName{};

// reading process based off of https://github.com/FabianFG/CUE4Parse/blob/master/CUE4Parse/MappingsProvider/Usmap/UsmapParser.cs
// if you want to dump your own mappings: https://github.com/OutTheShade/UnrealMappingsDumper

static __forceinline std::string& ReadName(FArchive& Ar, std::vector<std::string>& Names)
{
	int32_t NameIdx;
	Ar << NameIdx;

	if (NameIdx == -1)
		return InvalidName;

	return Names[NameIdx];
}

template <typename T>
static TObjectPtr<T> GetOrCreateObject(std::string& ClassName, TMap<std::string, UObjectPtr>& ObjectArray)
{
	if (ObjectArray.contains(ClassName))
	{
		return ObjectArray[ClassName].As<T>();
	}

	TObjectPtr<T> Ret = std::make_shared<T>();
	Ret->SetName(ClassName);

	ObjectArray.insert_or_assign(ClassName, Ret);
	return Ret;
}

enum class EUsmapVersion : uint8_t
{
	/** Initial format. */
	Initial,

	/** Adds package versioning to aid with compatibility */
	PackageVersioning,

	LatestPlusOne,
	Latest = LatestPlusOne - 1
};

enum class EUsmapCompressionMethod : uint8_t
{
	None,
	Oodle,
	Brotli,
	ZStandard,

	Unknown = 0xFF
};

class FPropertyFactory
{
	TMap<std::string, UObjectPtr>& ObjectArray;
	TMap<std::string, TSharedPtr<FReflectedEnum>> Enums;
	std::vector<std::string>& Names;

	FProperty* SerializePropertyInternal(FArchive& Ar)
	{
		EPropertyType Type;
		Ar.Serialize(&Type, sizeof(Type));

		FProperty* Ret = nullptr;

		switch (Type)
		{
		case EPropertyType::EnumProperty:
		{
			auto Prop = new FEnumProperty;
			Prop->UnderlyingProp = SerializePropertyInternal(Ar);
			Prop->Enum = Enums[ReadName(Ar, Names)];
			Ret = Prop;
			break;
		}
		case EPropertyType::StructProperty:
		{
			auto Prop = new FStructProperty;
			Prop->Struct = GetOrCreateObject<UClass>(ReadName(Ar, Names), ObjectArray);
			Ret = Prop;
			break;
		}
		case EPropertyType::ArrayProperty:
		{
			auto Prop = new FArrayProperty;
			Prop->ElementType = SerializePropertyInternal(Ar);
			Ret = Prop;
			break;
		}
		case EPropertyType::SetProperty:
		{
			auto Prop = new FSetProperty;
			Prop->ElementType = SerializePropertyInternal(Ar);
			Ret = Prop;
			break;
		}
		case EPropertyType::MapProperty:
		{
			auto Prop = new FMapProperty;
			Prop->KeyType = SerializePropertyInternal(Ar);
			Prop->ValueType = SerializePropertyInternal(Ar);
			Ret = Prop;
			break;
		}
		case EPropertyType::WeakObjectProperty:
		case EPropertyType::LazyObjectProperty:
		case EPropertyType::ObjectProperty: Ret = new FObjectProperty; break;
		case EPropertyType::ByteProperty: Ret = new FByteProperty; break;
		case EPropertyType::BoolProperty: Ret = new FBoolProperty; break;
		case EPropertyType::SoftObjectProperty: Ret = new FSoftObjectProperty; break;
		case EPropertyType::DelegateProperty: Ret = new FDelegateProperty; break;
		case EPropertyType::MulticastDelegateProperty: Ret = new FMulticastDelegateProperty; break;
		default: Ret = new FProperty; break;
		};

		Ret->Type = Type;

		return Ret;
	}

public:

	FPropertyFactory(std::vector<std::string>& InNames, TMap<std::string, UObjectPtr>& InObjectArray)
		: Names(InNames), ObjectArray(InObjectArray)
	{
	}

	void SerializeEnums(FArchive& Ar)
	{
		uint32_t EnumsCount;
		Ar << EnumsCount;

		Enums.reserve(EnumsCount);

		for (size_t i = 0; i < EnumsCount; i++)
		{
			auto& EnumName = ReadName(Ar, Names);

			uint8_t EnumNamesCount;
			Ar << EnumNamesCount;

			auto Enum = std::make_shared<FReflectedEnum>();
			Enum->EnumName = EnumName;

			auto& EnumNames = Enum->Enum;
			EnumNames.resize(EnumNamesCount);

			for (size_t j = 0; j < EnumNamesCount; j++)
			{
				EnumNames[j] = ReadName(Ar, Names);
			}

			Enums.insert_or_assign(EnumName, Enum);
		}
	}

	FProperty* SerializeProperty(FArchive& Ar) // TODO: make ustruct free properties
	{
		uint16_t Index;
		uint8_t ArrayDim;
		Ar << Index << ArrayDim;

		std::string& Name = ReadName(Ar, Names);

		auto Ret = SerializePropertyInternal(Ar);

		Ret->Name = Name;
		Ret->Index = Index;
		Ret->ArrayDim = ArrayDim;

		return Ret;
	}
};

bool Mappings::RegisterTypesFromUsmap(std::string Path, TMap<std::string, UObjectPtr>& ObjectArray) // TODO: try to save memory
{
	FFileReader FileAr(Path.c_str());

	if (!FileAr.IsValid())
	{
		Log<Error>("Could not open handle to usmap file or it does not exist. Returning.");
		return false;
	}

	uint16_t Magic;
	FileAr << Magic;

	if (Magic != USMAP_FILE_MAGIC)
	{
		Log<Error>("Invalid usmap file magic. Returning.");
		return false;
	}

	EUsmapVersion Ver;
	FileAr.Serialize(&Ver, sizeof(Ver));

	if (Ver < EUsmapVersion::Initial || Ver > EUsmapVersion::Latest)
	{
		Log<Error>("Invalid usmap file version. Returning.");
		return false;
	}

	// TODO: Versioning info https://github.com/FabianFG/CUE4Parse/blob/master/CUE4Parse/MappingsProvider/Usmap/UsmapParser.cs#L40

	EUsmapCompressionMethod CompressionMethod;
	FileAr.Serialize(&CompressionMethod, sizeof(EUsmapCompressionMethod));

	uint32_t CompressedSize, DecompressedSize;
	FileAr << CompressedSize << DecompressedSize;

	auto UsmapBuf = std::make_unique<uint8_t[]>(DecompressedSize);

	switch (CompressionMethod)
	{
	case EUsmapCompressionMethod::None:
	{
		if (CompressedSize != DecompressedSize)
		{
			Log<Error>("Usmap compression method is uncompressed but the compressed and decompressed size are different. Returning.");
			return false;
		}

		FileAr.Serialize(UsmapBuf.get(), DecompressedSize);
		break;
	}
	case EUsmapCompressionMethod::Oodle:
	{
		auto CompressedBuf = std::make_unique<uint8_t[]>(CompressedSize);
		FileAr.Serialize(CompressedBuf.get(), CompressedSize);

		Oodle::Decompress(CompressedBuf.get(), CompressedSize, UsmapBuf.get(), DecompressedSize);
		break;
	}
	// TODO: brotli and zstandard
	default:
	{
		Log<Error>("Invalid usmap compression method.");
		return false;
	}
	}

	auto Ar = FMemoryReader(UsmapBuf.get(), DecompressedSize);

	uint32_t NamesCount;
	Ar << NamesCount;

	std::vector<std::string> Names(NamesCount);

	for (size_t i = 0; i < NamesCount; i++)
	{
		auto& Str = Names[i];

		uint8_t Len;
		Ar << Len;

		Str.resize(Len);
		Ar.Serialize(&Str[0], Len);
	}

	FPropertyFactory Factory(Names, ObjectArray);

	Factory.SerializeEnums(Ar);

	uint32_t StructCount;
	Ar << StructCount;

	for (size_t i = 0; i < StructCount; i++)
	{
		auto& ClassName = ReadName(Ar, Names);

		auto Struct = GetOrCreateObject<UClass>(ClassName, ObjectArray);

		auto& SuperName = ReadName(Ar, Names);

		if (!SuperName.empty())
		{
			if (ObjectArray.contains(SuperName))
			{
				Struct->SetSuper(ObjectArray[SuperName].As<UStruct>());
			}
			else
			{
				UClassPtr Super = std::make_shared<UClass>();
				ObjectArray.insert_or_assign(SuperName, Super);

				Struct->SetSuper(Super);
			}
		}

		uint16_t PropCount, SerializablePropCount;
		Ar << PropCount << SerializablePropCount;

		auto& Properties = Struct->Properties;
		Properties.resize(PropCount);

		for (size_t i = 0; i < SerializablePropCount; i++)
		{
			auto Prop = Factory.SerializeProperty(Ar);

			for (auto j = 0; j < Prop->GetArrayDim(); j++)
			{
				Properties[Prop->GetIndex() + j] = Prop;
			}
		}
	}

	return true;
}