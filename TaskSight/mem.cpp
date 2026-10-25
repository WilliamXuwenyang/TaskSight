#include "mem.h"
#include <windows.h>
#include <psapi.h>

MemInfo GetMemInfo() {
    MEMORYSTATUSEX mem{};
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    MemInfo info{};
    info.totalGB = mem.ullTotalPhys / (1024.0 * 1024 * 1024);
    info.freeGB = mem.ullAvailPhys / (1024.0 * 1024 * 1024);
    info.usedGB = info.totalGB - info.freeGB;
    info.percent = mem.dwMemoryLoad;
    return info;
}