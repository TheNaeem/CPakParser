#include <iostream>
#include <chrono>

#include "../CPakParser/Dataminer.h"

int main()
{
    auto start = std::chrono::steady_clock::now();
    Dataminer::Initialize("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");
    auto end = std::chrono::steady_clock::now();

    printf("[=] Init Time: %.02f ms\n", (end - start).count() / 1000000.);
}