export module CPakParser.Math.IntPoint;

import <type_traits>;
import CPakParser.Serialization.FArchive;

export template <typename InIntType>
struct TIntPoint
{
	using IntType = InIntType;
	static_assert(std::is_integral_v<IntType>, "An integer type is required.");

	union
	{
		struct
		{
			/** Holds the point's x-coordinate. */
			IntType X;

			/** Holds the point's y-coordinate. */
			IntType Y;
		};

		IntType XY[2];
	};

	/** An integer point with zeroed values. */
	static const TIntPoint ZeroValue;

	/** An integer point with INDEX_NONE values. */
	static const TIntPoint NoneValue;

	TIntPoint() = default;

	TIntPoint(IntType InX, IntType InY)
		: X(InX)
		, Y(InY)
	{
	}

	TIntPoint(IntType InXY)
		: X(InXY)
		, Y(InXY)
	{
	}

	template <typename OtherIntType>
	explicit TIntPoint(TIntPoint<OtherIntType> Other)
		: X(IntCastChecked<IntType>(Other.X))
		, Y(IntCastChecked<IntType>(Other.Y))
	{
	}

	const IntType& operator()(int32_t PointIndex) const
	{
		return XY[PointIndex];
	}

	IntType& operator()(int32_t PointIndex)
	{
		return XY[PointIndex];
	}

	bool operator==(const TIntPoint& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}

	bool operator!=(const TIntPoint& Other) const
	{
		return (X != Other.X) || (Y != Other.Y);
	}

	TIntPoint& operator*=(IntType Scale)
	{
		X *= Scale;
		Y *= Scale;

		return *this;
	}

	TIntPoint& operator/=(IntType Divisor)
	{
		X /= Divisor;
		Y /= Divisor;

		return *this;
	}

	TIntPoint& operator+=(const TIntPoint& Other)
	{
		X += Other.X;
		Y += Other.Y;

		return *this;
	}

	TIntPoint& operator*=(const TIntPoint& Other)
	{
		X *= Other.X;
		Y *= Other.Y;

		return *this;
	}

	TIntPoint& operator-=(const TIntPoint& Other)
	{
		X -= Other.X;
		Y -= Other.Y;

		return *this;
	}

	TIntPoint& operator/=(const TIntPoint& Other)
	{
		X /= Other.X;
		Y /= Other.Y;

		return *this;
	}

	TIntPoint& operator=(const TIntPoint& Other)
	{
		X = Other.X;
		Y = Other.Y;

		return *this;
	}

	TIntPoint operator*(IntType Scale) const
	{
		return TIntPoint(*this) *= Scale;
	}

	TIntPoint operator/(IntType Divisor) const
	{
		return TIntPoint(*this) /= Divisor;
	}

	TIntPoint operator+(const TIntPoint& Other) const
	{
		return TIntPoint(*this) += Other;
	}

	TIntPoint operator-(const TIntPoint& Other) const
	{
		return TIntPoint(*this) -= Other;
	}

	TIntPoint operator*(const TIntPoint& Other) const
	{
		return TIntPoint(*this) *= Other;
	}

	TIntPoint operator/(const TIntPoint& Other) const
	{
		return TIntPoint(*this) /= Other;
	}

	IntType& operator[](IntType Index)
	{
		return ((Index == 0) ? X : Y);
	}

	IntType operator[](IntType Index) const
	{
		return ((Index == 0) ? X : Y);
	}

	IntType Size() const
	{
		int64_t LocalX64 = (int64_t)X;
		int64_t LocalY64 = (int64_t)Y;
		return IntType(sqrt(double(LocalX64 * LocalX64 + LocalY64 * LocalY64)));
	}
	IntType SizeSquared() const
	{
		return X * X + Y * Y;
	}

	static int32_t Num()
	{
		return 2;
	}

	friend FArchive& operator<<(FArchive& Ar, TIntPoint& Point)
	{
		return Ar << Point.X << Point.Y;
	}
};

export typedef TIntPoint<int32_t> FInt32Point;
export typedef TIntPoint<int64_t> FInt64Point;
export typedef TIntPoint<uint32_t> FUint32Point;
export typedef TIntPoint<uint64_t> FUint64Point;

export typedef FInt32Point FIntPoint;
export typedef FUint32Point FUintPoint;