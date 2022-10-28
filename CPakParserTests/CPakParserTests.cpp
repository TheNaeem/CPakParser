#include <iostream>
#include <fstream>

#include <Windows.h>

#include "Exceptions.h"

#include "Dataminer/Dataminer.h"

int main()
{
	SetUnhandledExceptionFilter(Exceptions::UnhandledHandler);

	Dataminer::Options::WithLogging(true);
	Dataminer::Options::WithOodleDecompressor("oo2core_9_win64.dll");

	auto Core = Dataminer("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");

	//auto MappingsTask = Core.LoadTypeMappingsAsync("D:\\N's Stuff\\Output\\.data\\++Fortnite+Release-22.20-CL-22545427-Android_oo.usmap");
	Core.Initialize();

	//MappingsTask.wait();

	//Core.Test("FortniteGame/Content/Athena/Items/Cosmetics/Characters/CID_478_Athena_Commando_F_WorldCup.uasset");

	Sleep(-1);
}