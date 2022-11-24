export module CPakParser.AI.Navigation;

import <cstdint>;
import CPakParser.Serialization.FArchive;

export struct FNavAgentSelector
{
	static const uint32_t InitializedBit = 0x80000000;

	explicit FNavAgentSelector(const uint32_t InBits = 0x7fffffff) : PackedBits(InBits)
	{
	}

	union
	{
		struct
		{
			uint32_t bSupportsAgent0 : 1;
			uint32_t bSupportsAgent1 : 1;
			uint32_t bSupportsAgent2 : 1;
			uint32_t bSupportsAgent3 : 1;
			uint32_t bSupportsAgent4 : 1;
			uint32_t bSupportsAgent5 : 1;
			uint32_t bSupportsAgent6 : 1;
			uint32_t bSupportsAgent7 : 1;
			uint32_t bSupportsAgent8 : 1;
			uint32_t bSupportsAgent9 : 1;
			uint32_t bSupportsAgent10 : 1;
			uint32_t bSupportsAgent11 : 1;
			uint32_t bSupportsAgent12 : 1;
			uint32_t bSupportsAgent13 : 1;
			uint32_t bSupportsAgent14 : 1;
			uint32_t bSupportsAgent15 : 1;
		};
		uint32_t PackedBits;
	};

	__forceinline bool Contains(int32_t AgentIndex) const
	{
		return (AgentIndex >= 0 && AgentIndex < 16) ? !!(PackedBits & (1 << AgentIndex)) : false;
	}

	__forceinline void Set(int32_t AgentIndex)
	{
		if (AgentIndex >= 0 && AgentIndex < 16)
		{
			PackedBits |= (1 << AgentIndex);
		}
	}

	__forceinline bool IsInitialized() const
	{
		return (PackedBits & InitializedBit) != 0;
	}

	__forceinline void MarkInitialized()
	{
		PackedBits |= InitializedBit;
	}

	__forceinline void Empty()
	{
		PackedBits = 0;
	}

	bool IsSame(const FNavAgentSelector& Other) const
	{
		return (~InitializedBit & PackedBits) == (~InitializedBit & Other.PackedBits);
	}

	uint32_t GetAgentBits() const
	{
		return (~InitializedBit & PackedBits);
	}

	friend FArchive& operator<<(FArchive& Ar, FNavAgentSelector& Val)
	{
		return Ar << Val.PackedBits;
	}
};