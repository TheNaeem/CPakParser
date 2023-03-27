# CPakParser

## This project is still very unfinished. 

No guarantee it will be worked on anytime soon, as I haven't worked on it in a long time. Really just here cause I put a ton of time into it.

*Quick example of usage*:

```cpp
	Dataminer::Options::WithOodleDecompressor("oo2core_9_win64.dll");

	auto Core = Dataminer("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Content\\Paks");
	Core.SetVersionUE5(1008);

	auto MappingsTask = Core.LoadTypeMappingsAsync("C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Binaries\\Win64\\Mappings.usmap");
	Core.Initialize();
	MappingsTask.wait();

	FPakDirectory Outfits = Core.GetDirectory("../../../FortniteGame/Content/Athena/Items/Cosmetics/Characters/"); // TODO: resolve mount point in function

	auto Start = std::chrono::steady_clock::now();
	int i = 0;

	for (auto& Outfit : Outfits)
	{
		auto Pkg = Core.LoadPackage(Outfit.second);

		if (!Pkg)
			continue;

		auto Object = Pkg->GetFirstExport();

		if (!Object)
			continue;

		i++;
	}

	auto End = std::chrono::steady_clock::now();

	printf("[=] Fully loaded %d AthenaCosmeticItemDefinition packages in %.02f ms\n", i, (End - Start).count() / 1000000.);
```

Plans:
* Make it usable
* Finish templates
* Texture exporting
* Manifest streaming support
* Verse asset support
