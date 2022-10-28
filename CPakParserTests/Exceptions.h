#pragma once

#include <cstdio>
#include <Windows.h>
#include <Dbghelp.h>
#include <shlobj.h>
#include <strsafe.h>
#pragma comment(lib, "DbgHelp")

namespace Exceptions
{
    static void MakeMiniDump(EXCEPTION_POINTERS* e)
    {
        char Name[MAX_PATH] { 0 };
        auto NameEnd = Name + GetModuleFileNameA(GetModuleHandleA(nullptr), Name, MAX_PATH);

        SYSTEMTIME t;
        GetSystemTime(&t);
        wsprintfA(NameEnd - strlen(".exe"), "_%4d%02d%02d_%02d%02d%02d.dmp", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

        HANDLE hFile = CreateFileA(Name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (hFile == INVALID_HANDLE_VALUE)
            return;

        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
        exceptionInfo.ThreadId = GetCurrentThreadId();
        exceptionInfo.ExceptionPointers = e;
        exceptionInfo.ClientPointers = FALSE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory | MiniDumpWithFullMemory),
            e ? &exceptionInfo : NULL,
            NULL,
            NULL);

        if (hFile)
        {
            CloseHandle(hFile);
            hFile = NULL;
        }
        return;
    }

    static long UnhandledHandler(EXCEPTION_POINTERS* e)
    {
        MakeMiniDump(e);
        return EXCEPTION_CONTINUE_SEARCH;
    }

}
