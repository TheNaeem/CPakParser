#include "PackageId.h"

#include "Serialization/Archives.h"
#include "Misc/Hashing/CityHash.h"
#include <algorithm>

FPackageId::FPackageId(std::string Name)
{
	std::wstring NameW(Name.begin(), Name.end());

	std::transform(NameW.begin(), NameW.end(), NameW.begin(),
		[](unsigned char c) { return std::tolower(c); });

	Id = CityHash64(reinterpret_cast<const char*>(NameW.data()), NameW.size() * 2 /* size of bytes */);
}

FArchive& operator<<(FArchive& Ar, FPackageId& Value)
{
	Ar << Value.Id;

	return Ar;
}