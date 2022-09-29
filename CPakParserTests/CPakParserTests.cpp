#include <iostream>

#include "../CPakParser/Dataminer.h"
#include "../CPakParser/GameFileManager.h"

int main()
{
    Dataminer::WithOodleCompressor("oo2core_9_win64.dll");

    Dataminer::SubmitKey("0x235F62E68E0205802D946747BCBF56EF28406B310936A31481D9DA94F28C4F76");
    Dataminer::Initialize("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");

    auto Start = std::chrono::steady_clock::now();
    Dataminer::Test("FortniteGame/Content/Localization/Fortnite_locchunk10/en/Fortnite_locchunk10.locres");
    auto End = std::chrono::steady_clock::now();

    printf("[=] Init Time: %.02f ms\n", (End - Start).count() / 1000000.);
}