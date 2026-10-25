#pragma once
#include <string>
#include <vector>

struct DiskInfo {
    std::string drive;
    double totalGB;
    double usedGB;
    double freeGB;
};

std::vector<DiskInfo> GetDiskInfo();