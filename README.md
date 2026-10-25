# TaskSight

Windows 控制台实时系统监控工具，纯 C++ 实现，无第三方依赖。

## 功能

- **CPU 使用率**：实时百分比 + 进度条
- **内存占用**：总量 / 已用 / 空闲，带进度条
- **磁盘空间**：各分区使用率与容量
- **网络流量**：实时下载 / 上传速度
- **进程列表**：按内存占用排序的 Top 10 进程
- **无闪烁刷新**：基于 `WriteConsoleOutput` 单缓冲渲染，控制台不闪屏

## 编译

需要 Visual Studio 2022 或更高版本，Windows 10/11。

1. 打开 `TaskSight.sln`
2. 直接生成（Ctrl+Shift+B）
3. 运行 `TaskSight.exe`

链接依赖：
- `iphlpapi.lib`（网络）
- `psapi.lib`（进程）

## 项目结构

```
TaskSight/
├── main.cpp          # 入口 + 渲染循环
├── cpu.h / cpu.cpp   # CPU 使用率采集
├── mem.h / mem.cpp   # 内存信息采集
├── disk.h / disk.cpp # 磁盘分区信息采集
├── net.h / net.cpp   # 网络流量采集
├── process.h / process.cpp # 进程列表采集
└── README.md

```

## 技术说明

- 使用 Windows API（`NtQuerySystemInformation`、`GetPerformanceInfo`、`GetDiskFreeSpaceEx`、`GetIfTable2`、`EnumProcesses`）采集系统数据
- 渲染层采用 `WriteConsoleOutputW` + UTF-8→UTF-16 转换，支持中文显示且不闪屏
- 刷新频率 1 秒/帧

## License

MIT
