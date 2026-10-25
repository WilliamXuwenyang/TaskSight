#pragma once

struct MemInfo {
    double totalGB;
    double usedGB;
    double freeGB;
    double percent;
};

MemInfo GetMemInfo();