export module CPakParser.Math.PerPlatform;

import CPakParser.Serialization.FArchive;

export template<typename _StructType, typename _ValueType>
struct TPerPlatformProperty
{
	typedef _ValueType ValueType;
	typedef _StructType StructType;

	_ValueType GetDefault() const
	{
		const _StructType* This = StaticCast<const _StructType*>(this);
		return This->Default;
	}

	_ValueType GetValue() const
	{
		{
			const _StructType* This = StaticCast<const _StructType*>(this);
			return This->Default;
		}
	}

	template<typename OStructType, typename OValueType>
	friend FArchive& operator<<(FArchive& Ar, TPerPlatformProperty<OStructType, OValueType>& Property)
	{
		bool bCooked = false;
		OStructType* This = static_cast<OStructType*>(&Property);
		Ar << bCooked;
		Ar << This->Default;

		return Ar;
	}
};

export struct FPerPlatformFloat : public TPerPlatformProperty<FPerPlatformFloat, float>
{
	float Default;

	FPerPlatformFloat()
	: Default(0.f)
	{
	}

	FPerPlatformFloat(float InDefaultValue)
	: Default(InDefaultValue)
	{
	}
};