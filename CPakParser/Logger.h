#pragma once

#include <iostream>
#include <fstream>

#include <spdlog/spdlog.h>

inline bool LOGGING_ENABLED = false;

enum LogType
{
	Debug,
	Error,
	Warning,
	Info,
	Success,
	None
};

template <LogType type, typename... Args>
constexpr void Log(const char* format, Args... args)
{
	if (!LOGGING_ENABLED)
		return;

	switch (type)
	{
	case Success:
	case Info:
	{
		spdlog::info(format, args...);
		break;
	}

	case Debug:
	{
		spdlog::debug(format, args...);
		break;
	}

	case Error:
	{
		spdlog::error(format, args...);
		break;
	}

	case Warning:
	{
		spdlog::warn(format, args...);
		break;
	}

	}
}

template <LogType type, typename... Args>
constexpr void Log(std::string format, Args... args)
{
	return Log<type>(format.c_str(), args...);
}