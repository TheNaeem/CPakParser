#include <iostream>
#include <fstream>
#include <chrono>

#include <Windows.h>

#include "Exceptions.h"

#include "Dataminer/Dataminer.h"

int main()
{
	SetUnhandledExceptionFilter(Exceptions::UnhandledHandler);

	Dataminer::Options::WithLogging(true);
	Dataminer::Options::WithOodleDecompressor("oo2core_9_win64.dll");

	auto Core = Dataminer("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");
	Core.SetVersionUE5(UE5_1);

	auto MappingsTask = Core.LoadTypeMappingsAsync("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Binaries\\Win64\\Mappings.usmap");
	Core.Initialize();
	MappingsTask.wait();

	auto Start = std::chrono::steady_clock::now();
	
	Core.Test("FortniteGame/Content/Athena/Items/Cosmetics/Characters/CID_664_Athena_Commando_M_Gummi.uasset");

	auto End = std::chrono::steady_clock::now();
	printf("[+] Exported Gummi in %.02f ms\n", (End - Start).count() / 1000000.);

	Sleep(-1);
}