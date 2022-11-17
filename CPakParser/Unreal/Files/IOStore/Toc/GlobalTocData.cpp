#include "GlobalTocData.h"

#include "Files/IOStore/Misc/IoStoreReader.h"
#include "Files/IOStore/Toc/IoStoreToc.h"

import CPakParser.Serialization.MemoryReader;

void FGlobalTocData::Serialize(TSharedPtr<FIoStoreReader> Reader)
{
	auto ChunkId = FIoChunkId(0, 0, EIoChunkType::ScriptObjects); // when in doubt, check the Describe function
	auto OaL = Reader->GetToc()->GetOffsetAndLength(ChunkId);

	auto ScriptObjectsBuffer = Reader->Read(OaL);

	if (!ScriptObjectsBuffer)
		return;

	auto ScriptObjectsReader = FMemoryReader(ScriptObjectsBuffer.get(), OaL.GetLength());

	NameMap.Serialize(ScriptObjectsReader, FMappedName::EType::Global);

	int32_t NumScriptObjects = 0;
	ScriptObjectsReader << NumScriptObjects;

	auto ScriptObjectEntries = (FScriptObjectEntry*)(ScriptObjectsBuffer.get() + ScriptObjectsReader.Tell());

	ScriptObjectByGlobalIdMap.reserve(NumScriptObjects);

	for (size_t i = 0; i < NumScriptObjects; i++)
	{
		auto& ScriptObjectEntry = ScriptObjectEntries[i];

		ScriptObjectByGlobalIdMap.insert_or_assign(ScriptObjectEntry.GlobalIndex, ScriptObjectEntry);
	}
}