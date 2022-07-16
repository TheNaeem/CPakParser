#include <iostream>
#include <chrono>

#include "../CPakParser/Dataminer.h"
#include "../CPakParser/GameFileManager.h"
#include <urlmon.h>
#pragma comment(lib,"urlmon.lib")

int main()
{
    auto start = std::chrono::steady_clock::now();

    Dataminer::SubmitKey("0x840A3C61B7BA7FDC58EAB092AC9F29D23229DA63C417F2F0ADD69F30F1B6980D");
    Dataminer::SubmitKey("0xA137CD7D320BA3326FC87AC4F7CCCFA54830E26ECC4D2F676B6F5BD02B30A4D7", "C0AD693BD109852AFC71C988E5C3589A");
    Dataminer::SubmitKey("0x0A986782A4AFA55F9BDBE5ACAC2E931D5947B6B6E59FF1A67A2E2D87B20EE955", "6686344942A2886FC4FD4D3763A4890D");
    Dataminer::SubmitKey("0x9969BF229309432951E22B936DF1328E38F74EF30D1AF3E92CD55DD511AB1C72", "1B8A454D42FF2E63D828ABF3B4A303C1");

    Dataminer::Initialize("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");
     
    auto end = std::chrono::steady_clock::now();

    printf("\n[=] Init Time: %.02f ms\n", (end - start).count() / 1000000.); 
}