export module CPakParser.Files.ExportState;

import CPakParser.Core.UObject;
import <string>;

export struct FExportState
{
	UObjectPtr TargetObject = nullptr;
	std::string TargetObjectName = {};
	bool LoadTargetOnly = false;
};