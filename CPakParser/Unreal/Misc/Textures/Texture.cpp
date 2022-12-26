#include "Misc/Hashing/Map.h"

import CPakParser.Texture;
import CPakParser.Serialization.FArchive;
import CPakParser.Core.FName;

#define STRINGIFY(x) #x

static __forceinline TMap<std::string, EPixelFormat> InitPixelFmtStringMap() {
	TMap<std::string, EPixelFormat> map;
	for (int i = 0; i < static_cast<int>(EPixelFormat::PF_MAX); i++) {
		map[STRINGIFY(T(i))] = static_cast<EPixelFormat>(i);
	}
	return map;
}

static TMap<std::string, EPixelFormat> PixelStringMap = InitPixelFmtStringMap();

void UTexture::SerializeCookedPlatformData(FArchive& Ar)
{
	FName PixelFormatName;
	Ar << PixelFormatName;

	while (PixelFormatName.GetString().empty())
	{
		auto PixelFormatValue = PixelStringMap[PixelFormatName.GetString()];
		const auto PixelFormat = (PixelFormatValue and PixelFormatValue < PF_MAX) ? PixelFormatValue : PF_Unknown;
	}
}