import CPakParser.Files.Loader;
import CPakParser.Package;
import CPakParser.Serialization.FArchive;

FLoader::FLoader(UPackage& InPackage) 
	: Package(InPackage), bHasSerializedPackageFileSummary(false), Reader(nullptr)
{
}

FLoader::~FLoader()
{
}

FSharedAr FLoader::CreateFileReader(FFileEntryInfo Entry, bool bMemoryPreload)
{
	if (!Entry.IsValid())
		return nullptr;

	return Entry.GetAssociatedFile()->CreateEntryArchive(Entry);;
}

__forceinline bool FLoader::IsValid()
{
	return Package.IsValid();
}

TSharedPtr<FLoader> FLoader::FromPackage(UPackage& Package)
{
	if (Package.HasLoader()) return Package.GetLoader();

	struct LoaderImpl : FLoader // i know its kind of iffy but its like this for a reason
	{
		LoaderImpl(UPackage& Pkg) : FLoader(Pkg)
		{
		}
	};

	return std::make_shared<LoaderImpl>(Package);
}

void FLoader::SerializePackageSummary()
{
	if (bHasSerializedPackageFileSummary)
		return;

	*Reader << Summary;
}

void FLoader::LoadAllObjects()
{
	/*if (!Reader)
		Reader = CreateFileReader(Package.GetPath());

	if (!Reader)
		return;

	SerializePackageSummary();*/
}
