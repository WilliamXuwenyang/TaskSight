#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include "net.h"
#include <windows.h>
#include <iphlpapi.h>
#include <cstdint>
#include <vector>

static uint64_t g_lastIn = 0, g_lastOut = 0;
static bool g_netInit = false;
static LARGE_INTEGER g_lastTime;

NetSpeed GetNetworkSpeed() {
    ULONG bufSize = 0;
    DWORD ret = GetIfTable(nullptr, &bufSize, FALSE);
    if (ret != ERROR_INSUFFICIENT_BUFFER) return NetSpeed{};

    std::vector<unsigned char> buf(bufSize);
    PMIB_IFTABLE table = reinterpret_cast<PMIB_IFTABLE>(buf.data());
    if (GetIfTable(table, &bufSize, FALSE) != NO_ERROR) return NetSpeed{};

    uint64_t totalIn = 0, totalOut = 0;
    for (DWORD i = 0; i < table->dwNumEntries; i++) {
        const MIB_IFROW& row = table->table[i];
        if (row.dwType == MIB_IF_TYPE_LOOPBACK) continue;
        if (row.dwOperStatus != MIB_IF_OPER_STATUS_OPERATIONAL) continue;
        totalIn += row.dwInOctets;
        totalOut += row.dwOutOctets;
    }

    LARGE_INTEGER now, freq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&freq);

    NetSpeed speed{};
    if (g_netInit) {
        double dt = (double)(now.QuadPart - g_lastTime.QuadPart) / freq.QuadPart;
        if (dt > 0) {
            int64_t inDiff = (int64_t)(totalIn - g_lastIn);
            int64_t outDiff = (int64_t)(totalOut - g_lastOut);
            if (inDiff > 0) speed.downloadBps = inDiff / dt;
            if (outDiff > 0) speed.uploadBps = outDiff / dt;
        }
    }

    g_lastIn = totalIn;
    g_lastOut = totalOut;
    g_lastTime = now;
    g_netInit = true;
    return speed;
}