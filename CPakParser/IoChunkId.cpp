#include "IoChunkId.h"
#include <stdlib.h>

#define NETWORK_ORDER16(x) _byteswap_ushort(x)

FIoChunkId::FIoChunkId(uint64_t ChunkId, uint16_t ChunkIndex, EIoChunkType IoChunkType)
{
	*reinterpret_cast<uint64_t*>(&Id[0]) = ChunkId;
	*reinterpret_cast<uint16_t*>(&Id[8]) = NETWORK_ORDER16(ChunkIndex);
	*reinterpret_cast<uint8_t*>(&Id[11]) = static_cast<uint8_t>(IoChunkType);
}