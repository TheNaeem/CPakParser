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

void FTexturePlatformData::Serialize(FArchive& Ar, UTexture* Owner)
{
	bool bUsingDerivedData = false;
	Ar.Serialize(&bUsingDerivedData, sizeof(bool));

	if (bUsingDerivedData)
	{
		// TODO: derived data serialization 
	}

	constexpr int64_t PlaceholderDerivedDataSize = 15;
	Ar.SeekCur(PlaceholderDerivedDataSize);

	Ar << SizeX;
	Ar << SizeY;
	Ar << PackedData;

	std::string PixelFormatString;
	Ar << PixelFormatString;

	PixelFormat = PixelStringMap[PixelFormatString];

	if (GetHasOptData())
	{
		OptData.Serialize(Ar);
	}

	int32_t FirstMipToSerialize = 0;
	Ar.SeekCur<int32_t>();

	int32_t NumMips = 0;
	Ar << NumMips;
}

void UTexture::SerializeCookedPlatformData(FArchive& Ar)
{
	FName PixelFormatName;
	Ar << PixelFormatName;

	PlatformData = FTexturePlatformData();

	while (!PixelFormatName.GetString().empty())
	{
		auto PixelFormatValue = PixelStringMap[PixelFormatName.GetString()];
		const auto PixelFormat = (PixelFormatValue and PixelFormatValue < PF_MAX) ? PixelFormatValue : PF_Unknown;

		auto SkipOffsetLoc = Ar.Tell();
		int64_t SkipOffset = 0;
		Ar << SkipOffset;

		if (PlatformData.PixelFormat == PF_Unknown)
		{
			PlatformData.Serialize(Ar, this);
		}
		else Ar.Seek(SkipOffsetLoc + SkipOffset);

		Ar << PixelFormatName;
	}
}