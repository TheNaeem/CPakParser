#include <iostream>
#include <fstream>

#include "../CPakParser/Dataminer.h"

int main()
{
    Dataminer::Options::WithLogging(true);
    Dataminer::Options::WithOodleDecompressor("oo2core_9_win64.dll");

    //Dataminer::SubmitKey("0x235F62E68E0205802D946747BCBF56EF28406B310936A31481D9DA94F28C4F76");
    auto Core = Dataminer("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");
    Core.Initialize();

    Core.Test("FortniteGame/Content/Athena/Items/Cosmetics/Characters/CID_478_Athena_Commando_F_WorldCup.uasset");

    std::cin.get();
}