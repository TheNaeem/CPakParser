#include "Core/Defines.h"
#include "Misc/Hashing/Map.h"

import CPakParser.Localization.Text;
import CPakParser.Logging;

typedef std::vector<std::pair<std::string, struct FFormatArgumentValue>> FFormatNamedArguments;
typedef std::vector<struct FFormatArgumentValue> FFormatOrderedArguments;

struct FNumberFormattingOptions
{
	FNumberFormattingOptions() = default;

	enum ERoundingMode
	{
		HalfToEven,
		HalfFromZero,
		HalfToZero,
		FromZero,
		ToZero,
		ToNegativeInfinity,
		ToPositiveInfinity
	};

	bool AlwaysSign;
	bool UseGrouping;
	ERoundingMode RoundingMode;
	int32_t MinimumIntegralDigits;
	int32_t MaximumIntegralDigits;
	int32_t MinimumFractionalDigits;
	int32_t MaximumFractionalDigits;

	friend void operator<<(FArchive& Ar, FNumberFormattingOptions& Value)
	{
		Ar << Value.AlwaysSign;
		Ar << Value.UseGrouping;

		int8_t ByteRoundingMode;
		Ar << ByteRoundingMode;
		Value.RoundingMode = static_cast<ERoundingMode>(ByteRoundingMode);

		Ar.Serialize(&Value.MinimumIntegralDigits, sizeof(int32_t) * 4);
	}
};

namespace EFormatArgumentType
{
	enum Type
	{
		Int,
		UInt,
		Float,
		Double,
		Text,
		Gender,
	};
}

namespace EDateTimeStyle
{
	enum Type
	{
		Default,
		Short,
		Medium,
		Long,
		Full,
		Custom
	};
}

struct FFormatArgumentValue
{
	friend void operator<<(FArchive& Ar, FFormatArgumentValue& Value)
	{
		int8_t TypeAsInt8;
		Ar << TypeAsInt8;

		Value.Type = (EFormatArgumentType::Type)TypeAsInt8;

		switch (Value.Type)
		{
		case EFormatArgumentType::Double:
		{
			Ar << Value.DoubleValue;
			break;
		}
		case EFormatArgumentType::Float:
		{
			Ar << Value.FloatValue;
			break;
		}
		case EFormatArgumentType::Int:
		{
			Ar << Value.IntValue;
			break;
		}
		case EFormatArgumentType::UInt:
		{
			Ar << Value.UIntValue;
			break;
		}
		case EFormatArgumentType::Text:
		{
			Value.TextValue = FText();
			Ar << Value.TextValue.value();
			break;
		}
		}
	}

	std::string ToString()
	{
		switch (Type)
		{
		case EFormatArgumentType::Double:
			return std::to_string(DoubleValue);
		case EFormatArgumentType::Float:
			return std::to_string(FloatValue);
		case EFormatArgumentType::Int:
			return std::to_string(IntValue);
		case EFormatArgumentType::UInt:
			return std::to_string(UIntValue);
		case EFormatArgumentType::Text:
			return TextValue.value().ToString();
		default: return {};
		}
	}

	EFormatArgumentType::Type Type;

	union
	{
		int64_t IntValue;
		uint64_t UIntValue;
		float FloatValue;
		double DoubleValue;
	};

	std::optional<FText> TextValue;
};

struct FFormatArgumentData
{
	FFormatArgumentData()
	{
		ResetValue();
	}

	void ResetValue()
	{
		ArgumentValueType = EFormatArgumentType::Text;
		ArgumentValue = {};
		ArgumentValueInt = 0;
		ArgumentValueFloat = 0.0f;
		ArgumentValueDouble = 0.0;
		ArgumentValueGender = ETextGender::Masculine;
	}

	friend void operator<<(FArchive& Ar, FFormatArgumentData& Value)
	{
		if (Ar.UEVer() >= VER_UE4_K2NODE_VAR_REFERENCEGUIDS) // There was no version bump for this change, but VER_UE4_K2NODE_VAR_REFERENCEGUIDS was made at almost the same time.
		{
			Ar << Value.ArgumentName;
		}
		else
		{
			FText TempValue;
			Ar << TempValue;
			Value.ArgumentName = TempValue.ToString();
		}

		Value.ResetValue();
		Value.ArgumentValueType = EFormatArgumentType::Text;

		switch (Value.ArgumentValueType)
		{
		case EFormatArgumentType::Int:
		{
			int32_t IntValue = static_cast<int32_t>(Value.ArgumentValueInt);
			Ar << IntValue;
			Value.ArgumentValueInt = static_cast<int64_t>(IntValue);
			break;
		}
		case EFormatArgumentType::Float:
			Ar << Value.ArgumentValueFloat;
			break;
		case EFormatArgumentType::Double:
			Ar << Value.ArgumentValueDouble;
			break;
		case EFormatArgumentType::Text:
			Ar << Value.ArgumentValue;
			break;
		case EFormatArgumentType::Gender:
		{
			uint8_t& Gender = (uint8_t&)Value.ArgumentValueGender;
			Ar << Gender;
			break;
		}
		default:
			break;
		}
	}

	std::string ArgumentName;
	EFormatArgumentType::Type ArgumentValueType;
	FText ArgumentValue;
	int64_t ArgumentValueInt;
	float ArgumentValueFloat;
	double ArgumentValueDouble;
	ETextGender ArgumentValueGender;
};

enum class ETextHistoryType : int8_t
{
	None = -1,
	Base = 0,
	NamedFormat,
	OrderedFormat,
	ArgumentFormat,
	AsNumber,
	AsPercent,
	AsCurrency,
	AsDate,
	AsTime,
	AsDateTime,
	Transform,
	StringTableEntry,
	TextGenerator,
};

class FTextBaseHistory : public ITextData
{
public:

	FTextBaseHistory() = default;

	FTextBaseHistory(std::string& InSourceString)
		: SourceString(InSourceString), TextId({})
	{
	}

	FTextBaseHistory(FTextId& InTextId, std::string& InSourceString)
		: TextId(InTextId), SourceString(InSourceString)
	{
	}

	__forceinline std::string& GetString() override
	{
		return SourceString;
	}

	__forceinline FTextId& GetTextId()
	{
		return TextId;
	}

	void Serialize(FArchive& Ar) override
	{
		FTextKey Namespace;
		Namespace.SerializeAsString(Ar);

		FTextKey Key;
		Key.SerializeAsString(Ar);

		Ar << SourceString;

		TextId = { Namespace, Key };
	}

private:

	FTextId TextId;
	std::string SourceString;
};

class FTextNamedFormat : public ITextData
{
public:

	__forceinline void Serialize(FArchive& Ar) override
	{
		Ar << Text;
		Ar << Arguments;
	}

	__forceinline std::string& GetString() override
	{
		return Text.ToString(); // TODO: return a formatted result
	}

	FText Text;
	FFormatNamedArguments Arguments;
};

class FTextOrderedFormat : public ITextData
{
public:

	__forceinline void Serialize(FArchive& Ar) override
	{
		Ar << Text;
		Ar << Arguments;
	}

	__forceinline std::string& GetString() override
	{
		return Text.ToString(); // TODO: return a formatted result
	}

	FText Text;
	FFormatOrderedArguments Arguments;
};

class FTextArgumentDataFormat : public ITextData
{
public:

	__forceinline void Serialize(FArchive& Ar) override
	{
		Ar << Text;
		Ar << Arguments;
	}

	__forceinline std::string& GetString() override
	{
		return Text.ToString(); // TODO: return a formatted result
	}

	FText Text;
	std::vector<FFormatArgumentData> Arguments;
};

class FTextFormatNumber : public ITextData
{
public:

	void Serialize(FArchive& Ar) override
	{
		FFormatArgumentValue SourceValTemp;
		Ar << SourceValTemp;
		SourceValue = SourceValTemp.ToString();

		bool bHasFormatOptions;
		Ar << bHasFormatOptions;

		if (bHasFormatOptions)
		{
			FNumberFormattingOptions Temp;
			Ar << Temp;
		}

		std::string CultureName;
		Ar << CultureName;
	}

	__forceinline std::string& GetString() override
	{
		return SourceValue; // TODO: return a formatted result
	}

	std::string SourceValue;
};

class FTextCurrency : public FTextFormatNumber
{
public:

	void Serialize(FArchive& Ar) override
	{
		if (Ar.UEVer() >= VER_UE4_ADDED_CURRENCY_CODE_TO_FTEXT)
		{
			Ar << CurrencyCode;
		}

		FTextFormatNumber::Serialize(Ar);
	}

	std::string CurrencyCode;
};

class FTextDate : public ITextData
{
public:

	void Serialize(FArchive& Ar) override
	{
		Ar << SourceDateTime;

		int8_t DateStyleInt8;
		Ar << DateStyleInt8;
		DateStyle = static_cast<EDateTimeStyle::Type>(DateStyleInt8);

		if (Ar.UEVer() >= VER_UE4_FTEXT_HISTORY_DATE_TIMEZONE)
		{
			Ar << TimeZone;
		}

		Ar << TargetCulture;
	}

	__forceinline std::string& GetString() override
	{
		static std::string Temp = "[CPakParser] TODO: Return a formatted datetime from FText.";
		return Temp; // TODO: return a formatted result
	}

	int64_t SourceDateTime;
	EDateTimeStyle::Type DateStyle;
	std::string TimeZone;
	std::string TargetCulture;
};

class FTextDateTime : public ITextData
{
public:

	void Serialize(FArchive& Ar) override
	{
		Ar << SourceDateTime;

		int8_t DateStyleInt8;
		Ar << DateStyleInt8;
		DateStyle = static_cast<EDateTimeStyle::Type>(DateStyleInt8);

		int8_t TimeStyleInt8;
		Ar << TimeStyleInt8;
		TimeStyle = static_cast<EDateTimeStyle::Type>(TimeStyleInt8);

		if (DateStyle == EDateTimeStyle::Custom)
		{
			Ar << CustomPattern;
		}

		Ar << TimeZone;
		Ar << TargetCulture;
	}

	__forceinline std::string& GetString() override
	{
		static std::string Temp = "[CPakParser] TODO: Return a formatted datetime from FText.";
		return Temp; // TODO: return a formatted result
	}

	int64_t SourceDateTime;
	EDateTimeStyle::Type DateStyle;
	EDateTimeStyle::Type TimeStyle;
	std::string CustomPattern;
	std::string TimeZone;
	std::string TargetCulture;
};

class FTextTransform : public ITextData
{
public:

	enum class ETransformType : uint8_t
	{
		ToLower = 0,
		ToUpper,
	};

	void Serialize(FArchive& Ar) override
	{
		Ar << SourceText;
		uint8_t& TransformTypeRef = (uint8_t&)TransformType;
		Ar << TransformTypeRef;
	}

	__forceinline std::string& GetString() override
	{
		return SourceText.ToString();
	}

	FText SourceText;
	ETransformType TransformType;
};

FArchive& operator<<(FArchive& Ar, FText& Value)
{
	if (Ar.UEVer() < VER_UE4_FTEXT_HISTORY)
	{
		std::string SourceStringToImplantIntoHistory;
		Ar << SourceStringToImplantIntoHistory;

		FTextId TextId;
		if (Ar.UEVer() >= VER_UE4_ADDED_NAMESPACE_AND_KEY_DATA_TO_FTEXT)
		{
			FTextKey Namespace, Key;
			Namespace.SerializeAsString(Ar);
			Key.SerializeAsString(Ar);

			TextId = { Namespace, Key };
		}

		Value.Data = std::make_unique<FTextBaseHistory>(TextId, SourceStringToImplantIntoHistory);
	}

	Ar << Value.Flags;

	if (Ar.UEVer() >= VER_UE4_FTEXT_HISTORY)
	{
		int8_t HistoryType;
		Ar << HistoryType;

		switch ((ETextHistoryType)HistoryType)
		{
		case ETextHistoryType::Base:
		{
			Value.Data = std::make_unique<FTextBaseHistory>();
			break;
		}
		case ETextHistoryType::NamedFormat:
		{
			Value.Data = std::make_unique<FTextNamedFormat>();
			break;
		}
		case ETextHistoryType::OrderedFormat:
		{
			Value.Data = std::make_unique<FTextOrderedFormat>();
			break;
		}
		case ETextHistoryType::ArgumentFormat:
		{
			Value.Data = std::make_unique<FTextArgumentDataFormat>();
			break;
		}
		case ETextHistoryType::AsPercent:
		case ETextHistoryType::AsNumber:
		{
			Value.Data = std::make_unique<FTextFormatNumber>();
			break;
		}
		case ETextHistoryType::AsCurrency:
		{
			Value.Data = std::make_unique<FTextCurrency>();
			break;
		}
		case ETextHistoryType::AsTime:
		case ETextHistoryType::AsDate:
		{
			Value.Data = std::make_unique<FTextDate>();
			break;
		}
		case ETextHistoryType::AsDateTime:
		{
			Value.Data = std::make_unique<FTextDateTime>();
			break;
		}
		case ETextHistoryType::Transform:
		{
			Value.Data = std::make_unique<FTextTransform>();
			break;
		}
		case ETextHistoryType::StringTableEntry:
		case ETextHistoryType::TextGenerator:
		{
			LogError("Unsupported Text history type. Wasn't gonna implement this until I actually encountered it.");
			return Ar;
		}
		default:
		{
			bool bHasCultureInvariantString = false;
			Ar << bHasCultureInvariantString;

			if (bHasCultureInvariantString)
			{
				std::string CultureInvariantString;
				Ar << CultureInvariantString;

				Value.Data = std::make_unique<FTextBaseHistory>(CultureInvariantString);
			}
			else Value.Data = std::make_unique<FTextBaseHistory>();

			return Ar;
		}
		}
	}

	Value.Data->Serialize(Ar);

	return Ar;
}

void FTextKey::Serialize(FArchive& Ar, ELocResVersion& Ver)
{
	if (Ver >= ELocResVersion::Optimized_CityHash64_UTF16)
	{
		Ar << StrHash;
	}
	else if (Ver == ELocResVersion::Optimized_CRC32)
	{
		Ar.SeekCur<uint32_t>();
	}

	Ar << Str;
}

void FTextKey::SerializeAsString(FArchive& Ar)
{
	Ar << Str;
}

FArchive& operator<<(FArchive& Ar, FTextLocalizationResourceString& A)
{
	Ar << A.String;
	Ar.SeekCur<uint32_t>();

	return Ar;
}

size_t hash_value(const FTextId& i)
{
	return phmap::HashState().combine(0, hash_value(i.Key), hash_value(i.Namespace));
}