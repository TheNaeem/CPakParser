export module CPakParser.Texture;

export import CPakParser.Core.UObject;
import CPakParser.Serialization.FArchive;
import <cstdint>;

export class FStripDataFlags
{
	__int8 GlobalStripFlags;
	__int8 ClassStripFlags;
};

export enum EPixelFormat : uint8_t
{
	PF_Unknown = 0,
	PF_A32B32G32R32F = 1,
	PF_B8G8R8A8 = 2,
	PF_G8 = 3,
	PF_G16 = 4,
	PF_DXT1 = 5,
	PF_DXT3 = 6,
	PF_DXT5 = 7,
	PF_UYVY = 8,
	PF_FloatRGB = 9,
	PF_FloatRGBA = 10,
	PF_DepthStencil = 11,
	PF_ShadowDepth = 12,
	PF_R32_FLOAT = 13,
	PF_G16R16 = 14,
	PF_G16R16F = 15,
	PF_G16R16F_FILTER = 16,
	PF_G32R32F = 17,
	PF_A2B10G10R10 = 18,
	PF_A16B16G16R16 = 19,
	PF_D24 = 20,
	PF_R16F = 21,
	PF_R16F_FILTER = 22,
	PF_BC5 = 23,
	PF_V8U8 = 24,
	PF_A1 = 25,
	PF_FloatR11G11B10 = 26,
	PF_A8 = 27,
	PF_R32_UINT = 28,
	PF_R32_SINT = 29,
	PF_PVRTC2 = 30,
	PF_PVRTC4 = 31,
	PF_R16_UINT = 32,
	PF_R16_SINT = 33,
	PF_R16G16B16A16_UINT = 34,
	PF_R16G16B16A16_SINT = 35,
	PF_R5G6B5_UNORM = 36,
	PF_R8G8B8A8 = 37,
	PF_A8R8G8B8 = 38,
	PF_BC4 = 39,
	PF_R8G8 = 40,
	PF_ATC_RGB = 41,
	PF_ATC_RGBA_E = 42,
	PF_ATC_RGBA_I = 43,
	PF_X24_G8 = 44,
	PF_ETC1 = 45,
	PF_ETC2_RGB = 46,
	PF_ETC2_RGBA = 47,
	PF_R32G32B32A32_UINT = 48,
	PF_R16G16_UINT = 49,
	PF_ASTC_4x4 = 50,
	PF_ASTC_6x6 = 51,
	PF_ASTC_8x8 = 52,
	PF_ASTC_10x10 = 53,
	PF_ASTC_12x12 = 54,
	PF_BC6H = 55,
	PF_BC7 = 56,
	PF_R8_UINT = 57,
	PF_L8 = 58,
	PF_XGXR8 = 59,
	PF_R8G8B8A8_UINT = 60,
	PF_R8G8B8A8_SNORM = 61,
	PF_R16G16B16A16_UNORM = 62,
	PF_R16G16B16A16_SNORM = 63,
	PF_PLATFORM_HDR_0 = 64,
	PF_PLATFORM_HDR_1 = 65,
	PF_PLATFORM_HDR_2 = 66,
	PF_NV12 = 67,
	PF_R32G32_UINT = 68,
	PF_ETC2_R11_EAC = 69,
	PF_ETC2_RG11_EAC = 70,
	PF_R8 = 71,
	PF_B5G5R5A1_UNORM = 72,
	PF_G16R16_SNORM = 78,
	PF_R8G8_UINT = 79,
	PF_R32G32B32_UINT = 80,
	PF_R32G32B32_SINT = 81,
	PF_R32G32B32F = 82,
	PF_R8_SINT = 83,
	PF_R64_UINT = 84,
	PF_R9G9B9EXP5 = 85,
	PF_MAX = 86
};

struct FTexturePlatformData
{
	int32_t SizeX;
	int32_t SizeY;
	uint32_t PackedData;
	EPixelFormat PixelFormat;

	FTexturePlatformData()
		: SizeX(0)
		, SizeY(0)
		, PackedData(0)
		, PixelFormat(PF_Unknown)
	{
	}

private:
	static constexpr uint32_t BitMask_CubeMap = 1u << 31u;
	static constexpr uint32_t BitMask_HasOptData = 1u << 30u;
	static constexpr uint32_t BitMask_NumSlices = BitMask_HasOptData - 1u;

public:

	void Serialize(FArchive& Ar, class UTexture* Owner);
	void SerializeCooked(FArchive& Ar, class UTexture* Owner, bool bStreamable);

	inline void SetPackedData(int32_t InNumSlices, bool bInHasOptData, bool bInCubeMap)
	{
		PackedData = (InNumSlices & BitMask_NumSlices) | (bInCubeMap ? BitMask_CubeMap : 0) | (bInHasOptData ? BitMask_HasOptData : 0);
	}

	inline bool GetHasOptData() const
	{
		return (PackedData & BitMask_HasOptData) == BitMask_HasOptData;
	}

	inline bool IsCubemap() const
	{
		return (PackedData & BitMask_CubeMap) == BitMask_CubeMap;
	}

	inline void SetIsCubemap(bool bCubemap)
	{
		PackedData = (bCubemap ? BitMask_CubeMap : 0) | (PackedData & (~BitMask_CubeMap));
	}

	inline int32_t GetNumSlices() const
	{
		return (int32_t)(PackedData & BitMask_NumSlices);
	}

	inline void SetNumSlices(int32_t NumSlices)
	{
		PackedData = (NumSlices & BitMask_NumSlices) | (PackedData & (~BitMask_NumSlices));
	}
};

export class UTexture : public UObject
{
private:

	FTexturePlatformData PlatformData;

public:

	void Serialize(FArchive& Ar) override
	{
		UObject::Serialize(Ar);

		Ar.SeekCur<FStripDataFlags>();
	}

	void SerializeCookedPlatformData(FArchive& Ar);
};