#include <iostream>
#include <fstream>
#include <chrono>

#include <Windows.h>

#include "Exceptions.h"
#include "Logger.h"

#include "Dataminer/Dataminer.h"


int main()
{
	SetUnhandledExceptionFilter(Exceptions::UnhandledHandler);

	Dataminer::Options::WithLogging(true);
	Dataminer::Options::WithOodleDecompressor("oo2core_9_win64.dll");

	auto Core = Dataminer("D:\\Fortnite\\FortniteGame\\Content\\Paks");

	Core.LoadTypeMappings("D:\\N's Stuff\\Output\\.data\\++Fortnite+Release-22.20-CL-22545427-Android_oo.usmap");
	Core.Initialize();
	auto t2 = std::chrono::high_resolution_clock::now();

	auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);

	std::chrono::duration<double, std::milli> ms_double = t2 - t1;
	printf("Initalized in %fms\n", ms_double.count());

	Core.Test("FortniteGame/Content/Athena/Items/Cosmetics/Characters/CID_478_Athena_Commando_F_WorldCup.uasset");

	Sleep(-1);
}