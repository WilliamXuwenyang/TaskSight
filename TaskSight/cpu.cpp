#include "cpu.h"
#include <windows.h>

static FILETIME g_lastIdle, g_lastKernel, g_lastUser;
static bool g_initialized = false;

static void GetTimes(FILETIME& idle, FILETIME& kernel, FILETIME& user) {
    GetSystemTimes(&idle, &kernel, &user);
}

static ULONGLONG FileTimeToULL(const FILETIME& ft) {
    ULARGE_INTEGER ul;
    ul.LowPart = ft.dwLowDateTime;
    ul.HighPart = ft.dwHighDateTime;
    return ul.QuadPart;
}

double GetCpuUsage() {
    FILETIME idle, kernel, user;
    GetTimes(idle, kernel, user);

    if (!g_initialized) {
        g_lastIdle = idle;
        g_lastKernel = kernel;
        g_lastUser = user;
        g_initialized = true;
        return 0.0;
    }

    double idleDiff = (double)(FileTimeToULL(idle) - FileTimeToULL(g_lastIdle));
    double kernelDiff = (double)(FileTimeToULL(kernel) - FileTimeToULL(g_lastKernel));
    double userDiff = (double)(FileTimeToULL(user) - FileTimeToULL(g_lastUser));

    g_lastIdle = idle;
    g_lastKernel = kernel;
    g_lastUser = user;

    double total = kernelDiff + userDiff;
    if (total == 0.0) return 0.0;

    return (total - idleDiff) / total * 100.0;
}