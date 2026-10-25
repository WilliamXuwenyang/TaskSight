#include "disk.h"
#include <windows.h>

std::vector<DiskInfo> GetDiskInfo() {
    std::vector<DiskInfo> result;
    DWORD drives = GetLogicalDrives();
    for (int i = 0; i < 26; i++) {
        if (!(drives & (1 << i))) continue;
        char root[4] = { (char)('A' + i), ':', '\\', '\0' };
        UINT type = GetDriveTypeA(root);
        if (type != DRIVE_FIXED && type != DRIVE_REMOVABLE) continue;

        ULARGE_INTEGER freeBytes, totalBytes, availBytes;
        if (!GetDiskFreeSpaceExA(root, &freeBytes, &totalBytes, &availBytes))
            continue;

        DiskInfo d;
        d.drive = std::string(1, 'A' + i) + ":";
        d.totalGB = totalBytes.QuadPart / (1024.0 * 1024 * 1024);
        d.freeGB = freeBytes.QuadPart / (1024.0 * 1024 * 1024);
        d.usedGB = d.totalGB - d.freeGB;
        result.push_back(d);
    }
    return result;
}