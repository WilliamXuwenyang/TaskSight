#pragma once
#include <string>
#include <vector>

struct ProcInfo {
    unsigned long pid;
    std::string name;
    double memMB;
};

std::vector<ProcInfo> GetTopProcesses(int topN);