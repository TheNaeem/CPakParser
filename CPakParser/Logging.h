#pragma once

#include <stdio.h>

inline bool LOGGING_ENABLED = false;

template <typename... Args>
__forceinline constexpr void Log(const char* Txt, Args... args)
{
	if (!LOGGING_ENABLED)
		return;

	printf("[\033[96mCPakParser\033[0m] ");
	printf(Txt, args...);
	printf("\n");
}

template <typename... Args>
__forceinline constexpr void LogError(const char* Txt, Args... args)
{
	if (!LOGGING_ENABLED)
		return;

	printf("[\033[96mCPakParser\033[0m] \033[101mERR\033[0m ");
	printf(Txt, args...);
	printf("\n");
}

template <typename... Args>
__forceinline constexpr void LogWarn(const char* Txt, Args... args)
{
	if (!LOGGING_ENABLED)
		return;

	printf("[\033[96mCPakParser\033[0m] \033[93mWARN\033[0m ");
	printf(Txt, args...);
	printf("\n");
}