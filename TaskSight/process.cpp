#include "process.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>
#include <string>

static std::string WideToNarrow(const wchar_t* w) {
    if (!w) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return std::string();
    std::string out(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w, -1, &out[0], size, nullptr, nullptr);
    return out;
}

std::vector<ProcInfo> GetTopProcesses(int topN) {
    std::vector<ProcInfo> procs;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return procs;

    PROCESSENTRY32 pe{};
    pe.dwSize = sizeof(pe);
    if (Process32First(snap, &pe)) {
        do {
            ProcInfo p;
            p.pid = pe.th32ProcessID;
            p.name = WideToNarrow(pe.szExeFile);
            p.memMB = 0;

            HANDLE hProc = OpenProcess(
                PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
            if (hProc) {
                PROCESS_MEMORY_COUNTERS pmc{};
                pmc.cb = sizeof(pmc);
                if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc))) {
                    p.memMB = pmc.WorkingSetSize / (1024.0 * 1024.0);
                }
                CloseHandle(hProc);
            }
            procs.push_back(p);
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);

    std::sort(procs.begin(), procs.end(),
        [](const ProcInfo& a, const ProcInfo& b) { return a.memMB > b.memMB; });

    if ((int)procs.size() > topN)
        procs.resize(topN);
    return procs;
}