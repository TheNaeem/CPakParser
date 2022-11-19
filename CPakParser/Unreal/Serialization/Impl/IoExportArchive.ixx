module;

#include "Core/Defines.h"

export module CPakParser.Serialization.IoExportArchive;

export import CPakParser.Serialization.FArchive;
import CPakParser.Zen.Data;
import CPakParser.Logging;
import CPakParser.Names.MappedName;
import CPakParser.Zen.Package;
import CPakParser.Package.Index;

export class FIoExportArchive : public FArchive
{
public:

	FIoExportArchive(uint8_t* AllExportDataPtr, uint8_t* CurrentExportPtr, FZenPackageData& InPackageData)
		: OriginalPos(AllExportDataPtr), CurrentPos(CurrentExportPtr), EndPos(AllExportDataPtr + InPackageData.ExportDataSize), PackageData(InPackageData)
	{
	}

	void ExportBufferBegin(uint64_t InExportCookedFileSerialOffset, uint64_t InExportSerialSize)
	{
		CookedSerialOffset = InExportCookedFileSerialOffset;
		BufferSerialOffset = CurrentPos - OriginalPos;
		CookedSerialSize = InExportSerialSize;
	}

	void ExportBufferEnd()
	{
		CookedSerialOffset = 0;
		BufferSerialOffset = 0;
		CookedSerialSize = 0;
	}

	void Skip(int64_t InBytes)
	{
		CurrentPos += InBytes;
	}

	virtual int64_t TotalSize() override
	{
		return PackageData.Header.PackageSummary.CookedHeaderSize + (EndPos - OriginalPos);
	}

	virtual int64_t Tell() override
	{
		int64_t Ret = (CurrentPos - OriginalPos);
		Ret -= BufferSerialOffset;
		Ret += CookedSerialOffset;
		return Ret;
	}

	virtual void Seek(int64_t Position) override
	{
		uint64_t BufferPosition = (uint64_t)Position;
		BufferPosition -= CookedSerialOffset;
		BufferPosition += BufferSerialOffset;
		CurrentPos = OriginalPos + BufferPosition;
	}

	virtual void Serialize(void* Data, int64_t Length) override
	{
		if (!Length)
		{
			return;
		}

		memcpyfst(Data, CurrentPos, Length);
		CurrentPos += Length;
	}

	inline virtual FArchive& operator<<(FName& Name) override
	{
		FArchive& Ar = *this;

		uint32_t NameIndex;
		Ar << NameIndex;
		uint32_t Number = 0;
		Ar << Number;

		auto MappedName = FMappedName::Create(NameIndex, Number, FMappedName::EType::Package);

		auto NameStr = PackageData.Header.NameMap.GetName(MappedName);

		if (NameStr.empty())
		{
			LogWarn("Name serialized with FIoExportArchive is empty or invalid");
		}

		Name = NameStr;

		return *this;
	}

	virtual FArchive& operator<<(UObjectPtr& Object) override
	{
		auto& Ar = *this;

		FPackageIndex Index;
		Ar << Index;

		if (Index.IsNull())
		{
			Object = nullptr;
			return *this;
		}

		if (Index.IsExport())
		{
			auto ExportIndex = Index.ToExport();
			if (ExportIndex < PackageData.Exports.size())
			{
				Object = PackageData.Exports[ExportIndex].Object;
			}
			else LogError("Export index read is not a valid index.");

			return *this;
		}

		auto& ImportMap = PackageData.Header.ImportMap;

		if (Index.IsImport() && Index.ToImport() < ImportMap.size())
		{
			Object = PackageData.Package->IndexToObject(
				PackageData.Header,
				PackageData.Exports,
				PackageData.Header.ImportMap[Index.ToImport()]);
		}
		else LogError("Bad object import index.");

		return *this;
	}

	FZenPackageData& PackageData;

private:

	uint8_t* OriginalPos;
	uint8_t* CurrentPos;
	uint8_t* EndPos;

	uint64_t CookedSerialSize = 0;
	uint64_t CookedSerialOffset = 0;
	uint64_t BufferSerialOffset = 0;
};