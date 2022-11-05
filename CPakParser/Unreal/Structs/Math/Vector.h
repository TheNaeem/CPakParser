#pragma once

struct FVector
{
	double X; 
	double Y; 
	double Z; 

	friend class FArchive& operator<<(class FArchive& Ar, FVector& Value);
};