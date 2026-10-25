#include <windows.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <csignal>
#include "cpu.h"
#include "mem.h"
#include "disk.h"
#include "net.h"
#include "process.h"

static HANDLE g_hOut = INVALID_HANDLE_VALUE;
static volatile bool g_running = true;

void HideCursor() {
    CONSOLE_CURSOR_INFO ci{};
    ci.dwSize = 1;
    ci.bVisible = FALSE;
    SetConsoleCursorInfo(g_hOut, &ci);
}

// 把一帧文本刷到控制台窗口，不闪屏
void RenderFrame(const std::string& text) {
    if (g_hOut == INVALID_HANDLE_VALUE) return;

    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (!GetConsoleScreenBufferInfo(g_hOut, &csbi)) return;

    SHORT w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    SHORT h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    if (w <= 0 || h <= 0) return;

    int cells = w * h;
    std::vector<CHAR_INFO> buf(cells);
    for (auto& c : buf) {
        c.Char.UnicodeChar = L' ';
        c.Attributes = 7;
    }

    int line = 0;
    std::istringstream iss(text);
    std::string row;
    while (line < h && std::getline(iss, row)) {
        // UTF-8 -> UTF-16
        int nw = MultiByteToWideChar(CP_UTF8, 0, row.c_str(), (int)row.size(), nullptr, 0);
        std::vector<wchar_t> wbuf(nw > 0 ? nw : 1);
        if (nw > 0)
            MultiByteToWideChar(CP_UTF8, 0, row.c_str(), (int)row.size(), wbuf.data(), nw);

        for (SHORT x = 0; x < w && x < (SHORT)wbuf.size(); ++x) {
            buf[line * w + x].Char.UnicodeChar = wbuf[x];
        }
        ++line;
    }

    COORD bufSize{ w, h };
    COORD bufCoord{ 0, 0 };
    SMALL_RECT region{ csbi.srWindow.Left, csbi.srWindow.Top,
                       csbi.srWindow.Right, csbi.srWindow.Bottom };

    WriteConsoleOutputW(g_hOut, buf.data(), bufSize, bufCoord, &region);
}

std::string FormatSpeed(double bps) {
    if (bps > 1024 * 1024) return std::to_string((int)(bps / 1024 / 1024)) + " MB/s";
    if (bps > 1024) return std::to_string((int)(bps / 1024)) + " KB/s";
    return std::to_string((int)bps) + " B/s";
}

std::string BuildBar(double percent, int width = 18) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    int filled = (int)(percent / 100.0 * width);
    if (filled > width) filled = width;
    std::string s = "  [";
    for (int i = 0; i < width; ++i)
        s += (i < filled ? (filled > width * 0.8 ? '!' : '#') : '.');
    s += "] ";
    std::ostringstream os;
    os << std::fixed << std::setprecision(1) << percent << "%";
    return s + os.str();
}

BOOL WINAPI CtrlHandler(DWORD type) {
    if (type == CTRL_C_EVENT || type == CTRL_CLOSE_EVENT) {
        g_running = false;
    }
    return TRUE;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    g_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HideCursor();
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    GetCpuUsage();
    GetNetworkSpeed();
    Sleep(500);

    while (g_running) {
        double cpu = GetCpuUsage();
        MemInfo mem = GetMemInfo();
        auto disks = GetDiskInfo();
        auto net = GetNetworkSpeed();
        auto procs = GetTopProcesses(10);

        std::ostringstream out;
        out << "  ====== TaskSight ======\n\n";

        out << "  CPU Usage:\n";
        out << BuildBar(cpu) << "\n\n";

        out << "  Memory:\n";
        out << BuildBar(mem.percent) << "\n";
        out << "    Total: " << std::fixed << std::setprecision(2) << mem.totalGB
            << " GB | Used: " << mem.usedGB << " GB | Free: " << mem.freeGB << " GB\n\n";

        out << "  Disk:\n";
        for (auto& d : disks) {
            double pct = d.totalGB > 0 ? (d.usedGB / d.totalGB * 100.0) : 0.0;
            out << "    " << d.drive << "  " << BuildBar(pct, 12)
                << "  (" << std::fixed << std::setprecision(1)
                << d.usedGB << "/" << d.totalGB << " GB)\n";
        }
        out << "\n";

        out << "  Network:\n";
        out << "    Download: " << FormatSpeed(net.downloadBps)
            << "    Upload: " << FormatSpeed(net.uploadBps) << "\n\n";

        out << "  Top Processes (by Memory):\n";
        out << "    " << std::left << std::setw(6) << "PID"
            << std::setw(20) << "Name"
            << std::right << std::setw(10) << "Mem(MB)" << "\n";
        for (auto& p : procs) {
            std::string nm = p.name.size() > 19 ? p.name.substr(0, 19) : p.name;
            out << "    " << std::left << std::setw(6) << p.pid
                << std::setw(20) << nm
                << std::right << std::setw(10) << std::fixed
                << std::setprecision(1) << p.memMB << "\n";
        }
        out << "\n  Press Ctrl+C to exit.\n";

        RenderFrame(out.str());
        Sleep(1000);
    }

    // 恢复光标
    CONSOLE_CURSOR_INFO ci{};
    ci.dwSize = 1;
    ci.bVisible = TRUE;
    SetConsoleCursorInfo(g_hOut, &ci);
    return 0;
}