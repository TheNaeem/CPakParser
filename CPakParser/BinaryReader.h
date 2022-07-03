#pragma once

#include <fstream>

/*
TODO: delete these once archives are fully implemented as there won't be use for these. I made these before I implemented archives so :/ 
*/

class ICPPReader
{
public:
	virtual __forceinline void Read(void* Dst, int Size) = 0;
	virtual __forceinline void Seek(size_t Offset) = 0;
	virtual __forceinline bool IsValid() = 0;
};

class BinaryReader : public ICPPReader
{
	std::ifstream m_Fs;

public:
	BinaryReader(const char* FileName) : m_Fs(FileName, std::ios::binary)
	{
	}

	~BinaryReader()
	{
		m_Fs.close();
	}
	
	__forceinline void Read(void* Dst, int Size) override
	{
		m_Fs.read((char*)Dst, Size);
	}

	template <typename T>
	__forceinline void Read(T* Dst)
	{
		return Read((void*)Dst, sizeof(T));
	}
	
	template <typename T>
	__forceinline T Read()
	{
		T ret; 
		Read<T>(&ret);
		return ret;
	}

	__forceinline void Seek(size_t Offset) override
	{
		m_Fs.seekg(Offset, m_Fs._Seekcur);
	}
	
	__forceinline bool IsValid() override
	{
		return !!m_Fs;
	}

	uint32_t Size() 
	{
		auto pos = m_Fs.tellg();
		m_Fs.seekg(0, m_Fs._Seekend);
		auto ret = m_Fs.tellg();
		m_Fs.seekg(pos, m_Fs._Seekbeg);
		return ret;
	}
};

class MemoryReader : public ICPPReader
{
	uint8_t* m_Buf;

public:

	MemoryReader(uint8_t* InPtr) : m_Buf(InPtr)
	{
	}

	__forceinline bool IsValid() override
	{
		return !!m_Buf;
	}

	__forceinline void Seek(size_t Offset) override
	{
		m_Buf += Offset;
	}

	__forceinline void Read(void* Dst, int Size) override
	{
		memcpy(Dst, m_Buf, Size);
		m_Buf += Size;
	}

	template <typename T>
	__forceinline T Read()
	{
		T Ret = *reinterpret_cast<T*>(m_Buf);
		m_Buf += sizeof(T);
		return Ret;
	}

	template <typename T>
	__forceinline std::vector<T> ReadArray(int ElementCount)
	{
		T*& Data = (T*&)m_Buf;

		auto Ret = std::vector<T>(Data, Data + ElementCount);

		Data += ElementCount;

		return Ret;
	}

	__forceinline uint8_t* GetBuffer()
	{
		return m_Buf;
	}
};