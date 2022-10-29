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

	Core.LoadTypeMappings("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Binaries\\Win64\\Mappings.usmap");

	auto Class = Core.GetObjectArray()["AthenaCharacterItemDefinition"];

	for (FPropertyIterator It(Class.As<UStruct>()); It; It.Next())
	{
		auto Prop = *It;

		std::cout << Prop->GetName() << std::endl;
	}

	//Core.Initialize();

	//Core.Test("FortniteGame/Content/Athena/Items/Cosmetics/Characters/CID_478_Athena_Commando_F_WorldCup.uasset");

	Sleep(-1);
}