#pragma once

struct FVector
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

	friend class FArchive& operator<<(class FArchive& Ar, FVector& Value);
};

struct FVector2D
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

	friend class FArchive& operator<<(class FArchive& Ar, FVector2D& Value);
};