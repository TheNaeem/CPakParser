#include <iostream>
#include <chrono>

#include "../CPakParser/Dataminer.h"
#include "../CPakParser/GameFileManager.h"

int main()
{
    auto start = std::chrono::steady_clock::now();

    Dataminer::SubmitKey("0x5311BD9BAE787F968F3870764131FB907C23D6BFAC4FA7F3AD4F2BF599CC5842");
    Dataminer::SubmitKey("0x12441D5E13C3F497BA069EDD97065D9645F64B485AAFABEA50E8CC205F6775F7", "1A9B01B59F609C0D7E9EF7887DA23087");

    Dataminer::Initialize("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");
     
    auto end = std::chrono::steady_clock::now();

    printf("\n[=] Init Time: %.02f ms\n", (end - start).count() / 1000000.); 
}