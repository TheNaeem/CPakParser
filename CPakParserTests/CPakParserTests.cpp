import CPakParser.Logging;

#include <iostream>
#include <fstream>
#include <chrono>

#include <Windows.h>

#include "Exceptions.h"

#include "Dataminer/Dataminer.h"


int main()
{
	//SetUnhandledExceptionFilter(Exceptions::UnhandledHandler);

	Dataminer::Options::WithLogging(true);
	Dataminer::Options::WithOodleDecompressor("oo2core_9_win64.dll");

	auto Core = Dataminer("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");
	Core.SetVersionUE5(UE5_1);

	auto MappingsTask = Core.LoadTypeMappingsAsync("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Binaries\\Win64\\Mappings.usmap");
	Core.Initialize();
	MappingsTask.wait();
	
	auto Characters = Core.TryGetDirectory("FortniteGame/Content/Athena/Items/Cosmetics/Characters/");

	if (!Characters.has_value())
	{
		printf("Could not find characters directory.\n");
		return 0;
	}

	auto Start = std::chrono::steady_clock::now();
	
	for (auto& Outfit : Characters.value())
	{
		std::cout << Outfit.first << std::endl;

		Core.Test(Outfit.second);
	}

	//Core.Test("FortniteGame/Content/Athena/Items/Cosmetics/Characters/Character_Billy.uasset");

	auto End = std::chrono::steady_clock::now();
	printf("[+] Done in %.02f ms\n", (End - Start).count() / 1000000.);

	Sleep(-1);
}