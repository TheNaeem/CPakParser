import CPakParser.Logging;
import CPakParser.Paths.SoftObjectPath;

#include <iostream>
#include <fstream>
#include <chrono>

#include <Windows.h>

#include "Dataminer/Dataminer.h"

int main()
{
	Dataminer::Options::WithLogging(false);
	Dataminer::Options::WithOodleDecompressor("oo2core_9_win64.dll");

	auto Core = Dataminer("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");
	Core.SetVersionUE5(1008);

	auto MappingsTask = Core.LoadTypeMappingsAsync("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Binaries\\Win64\\Mappings.usmap");
	Core.Initialize();
	MappingsTask.wait();

	auto Obj = Core.LoadObject("FortniteGame/Content/Athena/Items/Cosmetics/Characters/Character_Ballerina");

	Sleep(-1);
}