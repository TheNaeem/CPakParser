export module CPakParser.Math.FVector;

import CPakParser.Serialization.FArchive;

// TODO: add useful member functions

export struct FVector
{
	FVector() = default;

	__forceinline FVector(double InX, double InY, double InZ)
		: X(InX), Y(InY), Z(InZ)
	{
	}

	__forceinline FVector(float InX, float InY, float InZ)
		: X(InX), Y(InY), Z(InZ)
	{
	}

	double X;
	double Y;
	double Z;

	friend FArchive& operator<<(FArchive& Ar, FVector& Value)
	{
		if (Ar.UEVer() >= EUnrealEngineObjectUE5Version::LARGE_WORLD_COORDINATES)
		{
			Ar.Serialize(&Value, sizeof(FVector));
		}
		else
		{
			float X, Y, Z;
			Ar << X << Y << Z;

		}

		return Ar;
	}
};

export struct FVector2D
{
	FVector2D() = default;

	__forceinline FVector2D(double InX, double InY)
		: X(InX), Y(InY)
	{
	}

	__forceinline FVector2D(float InX, float InY)
		: X(InX), Y(InY)
	{
	}

	double X;
	double Y;

	friend FArchive& operator<<(FArchive& Ar, FVector2D& V)
	{
		if (Ar.UEVer() >= EUnrealEngineObjectUE5Version::LARGE_WORLD_COORDINATES)
		{
			Ar.Serialize(&V, sizeof(FVector2D));
		}
		else
		{
			float X, Y;
			Ar << X << Y;
			V = FVector2D(X, Y);
		}

		return Ar;
	}
};

export struct FVector4
{
	FVector4(double InX = 0.0f, double InY = 0.0f, double InZ = 0.0f, double InW = 1.0f)
		: X(InX)
		, Y(InY)
		, Z(InZ)
		, W(InW)
	{
	}

	union
	{
		struct
		{
			/** The vector's X-component. */
			double X;

			/** The vector's Y-component. */
			double Y;

			/** The vector's Z-component. */
			double Z;

			/** The vector's W-component. */
			double W;
		};

		double XYZW[4];
	};

	friend inline FArchive& operator<<(FArchive& Ar, FVector4& V)
	{
		if (Ar.UEVer() >= EUnrealEngineObjectUE5Version::LARGE_WORLD_COORDINATES)
		{
			Ar << V.X << V.Y << V.Z << V.W;
		}
		else
		{
			float X, Y, Z, W;
			Ar << X << Y << Z << W;
			V = FVector4(X, Y, Z, W);
		}
		return Ar;
	}
};