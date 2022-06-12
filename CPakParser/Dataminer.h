#pragma once

class Dataminer
{
	Dataminer()
	{
	}

public:
	Dataminer& operator=(const Dataminer&) = delete;
	Dataminer(const Dataminer&) = delete; 

	static Dataminer& Get()
	{
		static Dataminer Instance;
		return Instance;
	}
};