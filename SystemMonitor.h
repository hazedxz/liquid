#pragma once
#include <windows.h>
#include <string>

class HardwareMonitor {
private:
    ULARGE_INTEGER lastIdle, lastKernel, lastUser;

    void GetSystemTimesInfo(ULARGE_INTEGER& idle, ULARGE_INTEGER& kernel, ULARGE_INTEGER& user) {
        FILETIME ftIdle, ftKernel, ftUser;
        GetSystemTimes(&ftIdle, &ftKernel, &ftUser);
        idle.LowPart = ftIdle.dwLowDateTime;
        idle.HighPart = ftIdle.dwHighDateTime;
        kernel.LowPart = ftKernel.dwLowDateTime;
        kernel.HighPart = ftKernel.dwHighDateTime;
        user.LowPart = ftUser.dwLowDateTime;
        user.HighPart = ftUser.dwHighDateTime;
    }

public:
    HardwareMonitor() {
        GetSystemTimesInfo(lastIdle, lastKernel, lastUser);
    }

    int GetCPUUsage() {
        ULARGE_INTEGER idle, kernel, user;
        GetSystemTimesInfo(idle, kernel, user);

        ULONGLONG diffIdle = idle.QuadPart - lastIdle.QuadPart;
        ULONGLONG diffKernel = kernel.QuadPart - lastKernel.QuadPart;
        ULONGLONG diffUser = user.QuadPart - lastUser.QuadPart;

        ULONGLONG totalSys = diffKernel + diffUser;
        double percent = 0.0;
        
        if (totalSys > 0) {
            percent = (totalSys - diffIdle) * 100.0 / totalSys;
        }

        lastIdle = idle;
        lastKernel = kernel;
        lastUser = user;

        if (percent < 0.0) percent = 0.0;
        if (percent > 100.0) percent = 100.0;
        
        return (int)percent;
    }

    int GetRAMUsage() {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        return memInfo.dwMemoryLoad;
    }
};