#pragma once

#include <fstream>

class BinaryReader
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

	template <typename T>
	__forceinline T Read()
	{
		T ret; 
		m_Fs.read((char*)&ret, sizeof(T));
		return ret;
	}

	template <typename T>
	__forceinline void Read(T* Dst)
	{
		return Read((void*)Dst, sizeof T);
	}

	__forceinline void Read(void* Dst, int Size)
	{
		m_Fs.read((char*)Dst, Size);
	}

	__forceinline void Seek(size_t Offset, std::_Iosb<int>::_Seekdir Pos = std::ifstream::_Seekcur)
	{
		m_Fs.seekg(Offset, Pos);
	}
	
	__forceinline bool IsValid()
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

#define PakBinaryReader BinaryReader 