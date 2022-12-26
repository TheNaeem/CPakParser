export module CPakParser.Texture2D;

export import CPakParser.Texture;

export class UTexture2D : public UTexture
{
public:

	void Serialize(FArchive& Ar) override
	{
		UTexture::Serialize(Ar);

		Ar.SeekCur<FStripDataFlags>();

		bool bCooked;
		Ar << bCooked;

		if (bCooked)
		{
			SerializeCookedPlatformData(Ar);
		}
	}
};