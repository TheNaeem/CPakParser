import CPakParser.Logging;
import CPakParser.Paths.SoftObjectPath;
import CPakParser.Texture2D;

#include <iostream>
#include <fstream>
#include <chrono>

#include <Windows.h>

#include "Dataminer/Dataminer.h"

int main()
{
	Dataminer::Options::WithLogging(true);
	Dataminer::Options::WithOodleDecompressor("oo2core_9_win64.dll");

	auto Core = Dataminer("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");
	Core.SetVersionUE5(1008);

	auto MappingsTask = Core.LoadTypeMappingsAsync("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Binaries\\Win64\\Mappings.usmap");
	Core.Initialize();
	MappingsTask.wait();

	FPakDirectory Outfits = Core.GetDirectory("../../../FortniteGame/Content/Athena/Items/Cosmetics/Characters/"); // TODO: resolve mount point in function

	for (auto& Outfit : Outfits)
	{
		auto Pkg = Core.LoadPackage(Outfit.second);

		if (!Pkg)
			continue;

		auto Object = Pkg->GetFirstExport();

		if (!Object)
			continue;

		Log("Loaded %s", Object->GetName().c_str());
	}

	Sleep(-1);
}