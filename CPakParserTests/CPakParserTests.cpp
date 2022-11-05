#include <iostream>
#include <fstream>
#include <chrono>

#include <Windows.h>

#include "Exceptions.h"
#include "Logger.h"

#include "Dataminer/Dataminer.h"
#include "Core/Reflection/PropertyIterator.h"


int main()
{
	SetUnhandledExceptionFilter(Exceptions::UnhandledHandler);

	Dataminer::Options::WithLogging(true);
	Dataminer::Options::WithOodleDecompressor("oo2core_9_win64.dll");

	auto Core = Dataminer("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");
	Core.SetVersionUE5(UE5_1);

	auto MappingsTask = Core.LoadTypeMappingsAsync("C:\\Users\\zkana\\Downloads\\FortniteRelease-22.20-CL-22545427-Android_oo.usmap");
	Core.Initialize();
	MappingsTask.wait();

	Core.Test("FortniteGame/Content/Athena/Items/Cosmetics/Characters/CID_246_Athena_Commando_F_Grave.uasset");

	Sleep(-1);
}