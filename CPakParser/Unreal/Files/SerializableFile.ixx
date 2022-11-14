export module SerializableFile;

export import FArchiveBase;

export struct ISerializableFile
{
	virtual void Serialize(FArchive& Ar) = 0;
};