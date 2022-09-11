#include <iostream>

#include "../CPakParser/Dataminer.h"
#include "../CPakParser/GameFileManager.h"

int main()
{
    Dataminer::SubmitKey("0x5311BD9BAE787F968F3870764131FB907C23D6BFAC4FA7F3AD4F2BF599CC5842");
    Dataminer::Initialize("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");

    Dataminer::Test("../../../FortniteGame/Content/Localization/Fortnite_locchunk10/en/", "Fortnite_locchunk10.locres");
}