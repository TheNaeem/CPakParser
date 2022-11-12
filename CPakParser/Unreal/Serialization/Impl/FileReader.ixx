export module FileReader;

export import FArchiveBase;
import <fstream>;

export class FFileReader : public FArchive
{
public:

	FFileReader(const char* InFilename) : FileStream(InFilename, std::ios::binary)
	{
	}

	~FFileReader()
	{
		FileStream.close();
	}

	void Seek(int64_t InPos)
	{
		FileStream.seekg(InPos, FileStream._Seekbeg);
	}

	int64_t Tell()
	{
		return FileStream.tellg();
	}

	int64_t TotalSize()
	{
		auto Pos = FileStream.tellg();
		FileStream.seekg(0, FileStream._Seekend);

		auto Ret = FileStream.tellg();
		FileStream.seekg(Pos, FileStream._Seekbeg);

		return Ret;
	}

	bool Close()
	{
		FileStream.close();

		return !FileStream.is_open();
	}

	void Serialize(void* V, int64_t Length)
	{
		FileStream.read(static_cast<char*>(V), Length);
	}

	bool IsValid()
	{
		return !!FileStream;
	}

protected:

	std::ifstream FileStream;
};