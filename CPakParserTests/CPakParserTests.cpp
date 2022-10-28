#include <iostream>
#include <fstream>

#include "Dataminer/Dataminer.h"

int main()
{
	Dataminer::Options::WithLogging(true);
	Dataminer::Options::WithOodleDecompressor("oo2core_9_win64.dll");

	auto Core = Dataminer("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");

	Core.Initialize();


	/* Core.Initialize();

	 auto Start = std::chrono::steady_clock::now();

	 Core.Test("FortniteGame/Content/Athena/Items/Cosmetics/Characters/CID_478_Athena_Commando_F_WorldCup.uasset");

	 auto End = std::chrono::steady_clock::now();

	 printf("\nTime: %.02f ms\n", (End - Start).count() / 1000000.);*/

}