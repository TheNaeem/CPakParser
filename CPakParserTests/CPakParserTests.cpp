#include <iostream>
#include <fstream>

#include "../CPakParser/Dataminer.h"

int main()
{
    Dataminer::WithOodleDecompressor("oo2core_9_win64.dll");

    //Dataminer::SubmitKey("0x235F62E68E0205802D946747BCBF56EF28406B310936A31481D9DA94F28C4F76");
    Dataminer::Initialize("D:\\Fortnite\\FortniteGame\\Content\\Paks");

    Dataminer::Test("FortniteGame/Content/Athena/Items/Cosmetics/Characters/CID_478_Athena_Commando_F_WorldCup.uasset");
}