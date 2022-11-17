export module CPakParser.Files.SerializableFile;

export import CPakParser.Serialization.FArchive;

export struct ISerializableFile
{
	virtual void Serialize(FArchive& Ar) = 0;
};