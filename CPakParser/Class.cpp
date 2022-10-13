#include "Class.h"

UStructPtr UStruct::GetSuper()
{
	return Super;
}

void UStruct::SetSuper(UStructPtr Val)
{
	Super = Val;
}