export module CPakParser.Misc.BulkData;

import <cstdint>;
import CPakParser.Serialization.FArchive;
import CPakParser.Core.UObject;

export class FBulkData
{
public:

	FBulkData() = default;

	void Serialize(FArchive& Ar, UObject* Owner, int32_t ElemSize)
	{

	}
};

export template<typename ElementType>
class TBulkData : public FBulkData
{
public:

	int32_t GetElementSize() const
	{
		return sizeof(ElementType);
	}

	void Serialize(FArchive& Ar, UObject* Owner)
	{
		FBulkData::Serialize(Ar, Owner, GetElementSize());
	}
};

export using FByteBulkData = TBulkData<uint8_t>;