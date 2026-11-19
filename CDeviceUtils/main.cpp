#include <iostream>
#include "Win32Utils/CDeviceHelper.h"
#include "Win32Utils/CWmiHelper.h"
#include "Win32Utils/CStrUtils.h"
#include "CPerformHelper.h"
#include <Windows.h>
#include <iostream>

int main()
{
    setlocale(LC_ALL, "");

    std::vector<VIDEO_ADAPTER_DESC_INFO> vResult = CDeviceHelper::GetAllVideoAdapterDesc();

    CDeviceHelper obj;
    obj.Initialize();

    clock_t tmBegin = ::clock();
    DEVICE_INFO info = obj.GetDeviceInfo();
    clock_t tmEnd = ::clock();

    CPerformHelper perfmon;

    int nTimeInterval = 1000;

    perfmon.Initialize();
    perfmon.SetCollectInterval(nTimeInterval);
    perfmon.AddCounter(_T(PERFM_PATH_CPU_UTILITY));
    perfmon.AddCounter(_T(PERFM_PATH_DISK_READ_RATE));
    perfmon.AddCounter(_T(PERFM_PATH_DISK_WRITE_RATE));
    perfmon.AddCounter(_T(PERFM_PATH_CPU_PERFORMANCE));
    perfmon.AddCounter(_T(PERFM_PATH_CPU_FREQUENCY));
    perfmon.AddCounter(_T(PERFM_PATH_NETWORK_RECV_RATE));
    perfmon.AddCounter(_T(PERFM_PATH_NETWORK_SENT_RATE));
    perfmon.AddCounter(_T(PERFM_PATH_GPU_UTILITY));
    perfmon.AddCounter(_T(PERFM_PATH_GPU_3D_UTILITY));
    perfmon.AddCounter(_T(PERFM_PATH_GPU_COPY_UTILITY));
    perfmon.AddCounter(_T(PERFM_PATH_GPU_MEMORY_DEDICATED_USAGE_UTILITY));
    perfmon.AddCounter(_T(PERFM_PATH_GPU_MEMORY_SHARE_USAGE_UTILITY));
    perfmon.AddCounter(_T(PERFM_PATH_GPU_MEMORY_TOTAL_COMMITTED_USAGE_UTILITY));
    perfmon.AddCounter(_T(PERFM_PATH_PROCESS_V2_PROCESSOR_TIME));
    perfmon.AddCounter(_T(PERFM_PATH_PROCESS_V2_USER_TIME));
    perfmon.AddCounter(_T(PERFM_PATH_CPU_ACTUAL_FREQUENCY));
    perfmon.AddCounter(_T(PERFM_PATH_MEMORY_AVAILABLE_BYTES));
    perfmon.AddCounter(_T(PERFM_PATH_MEMORY_AVAILABLE_KBYTES));
    perfmon.AddCounter(_T(PERFM_PATH_MEMORY_AVAILABLE_MBYTES));
    perfmon.StartCollect();

    PDH_FMT_COUNTERVALUE value = { 0 };

    SIZE_T DedicatedVideoMemory = vResult[0].AdapterDesc.DedicatedVideoMemory;
    SIZE_T DedicatedSystemMemory = vResult[0].AdapterDesc.DedicatedSystemMemory;
    SIZE_T SharedSystemMemory = vResult[0].AdapterDesc.SharedSystemMemory;

    MEMORYSTATUSEX memStatus = { 0 };
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);

    while (true)
    {
        ::GlobalMemoryStatusEx(&memStatus);
        CStrUtils::ConsoleSetPos(0, 0);
        CStrUtils::ConsoleClear();

        // 获取CPU占用率
        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_CPU_UTILITY), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("CPU 利用率: %.2lf%% "), value.doubleValue);
            //CStrUtils::ConsolePrintf(_T("\r\n"));
        }

        // 获取CPU性能
        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_CPU_PERFORMANCE), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("性能: %.2lf%%"), value.doubleValue);
            CStrUtils::ConsolePrintf(_T("\r\n"));
        }

        // CPU实时频率
        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_CPU_ACTUAL_FREQUENCY), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("实时频率: %g Mhz "), value.doubleValue);
        }

        // 获取CPU性能
        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_CPU_FREQUENCY), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("当前速度: %g Mhz "), value.doubleValue);
        }

        CStrUtils::ConsolePrintf(_T("最大速度: %d Mhz "), info.Cpu.MaxClockSpeed);
        CStrUtils::ConsolePrintf(_T("\r\n"));

        // 物理内存
        CStrUtils::ConsolePrintf(_T("内存: %.1lf"), CStrUtils::FormatByteSize(memStatus.ullTotalPhys - memStatus.ullAvailPhys, CStrUtils::eUnitType::eUT_B, CStrUtils::eUnitType::eUT_GB).value);
        CStrUtils::ConsolePrintf(_T("/"));
        CStrUtils::ConsolePrintf(_T("%s "), CStrUtils::FormatByteSize(memStatus.ullTotalPhys, CStrUtils::eUnitType::eUT_B, CStrUtils::eUnitType::eUT_GB).strOutput.c_str());
        CStrUtils::ConsolePrintf(_T("%.2lf%%"), (double)(memStatus.ullTotalPhys - memStatus.ullAvailPhys) * 100 /(double)memStatus.ullTotalPhys);
        CStrUtils::ConsolePrintf(_T("\r\n"));

        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_MEMORY_AVAILABLE_MBYTES), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("可用内存: %g MB "), value.doubleValue);
        }

        CStrUtils::ConsolePrintf(_T("\r\n"));

        // 获取GPU占用率
        CStrUtils::ConsolePrintf(_T("GPU: "), value.doubleValue * 100);

        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_GPU_3D_UTILITY), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("3D: %g%% "), value.doubleValue * 100);
        }

        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_GPU_COPY_UTILITY), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("COPY: %g%% "), value.doubleValue * 100);
        }

        CStrUtils::ConsolePrintf(_T("\r\n"));

        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_GPU_MEMORY_DEDICATED_USAGE_UTILITY), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("GPU 专用内存: %0.1lf"), CStrUtils::FormatByteSize(value.doubleValue, CStrUtils::eUnitType::eUT_B, CStrUtils::eUnitType::eUT_GB).value);
            CStrUtils::ConsolePrintf(_T("/%s"), CStrUtils::FormatByteSize(DedicatedVideoMemory).strOutput.c_str());
            CStrUtils::ConsolePrintf(_T("\r\n"));
        }

        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_GPU_MEMORY_SHARE_USAGE_UTILITY), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("GPU 共享内存: %0.1lf"), CStrUtils::FormatByteSize(value.doubleValue, CStrUtils::eUnitType::eUT_B, CStrUtils::eUnitType::eUT_GB).value);
            CStrUtils::ConsolePrintf(_T("/%s"), CStrUtils::FormatByteSize(SharedSystemMemory).strOutput.c_str());
            CStrUtils::ConsolePrintf(_T("\r\n"));
        }


        //获取硬盘读速度
        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_DISK_READ_RATE), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("磁盘 读取: %s/s "), CStrUtils::FormatByteSize(value.doubleValue).strOutput.c_str());
        }

        //获取硬盘写速度
        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_DISK_WRITE_RATE), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("磁盘 写入: %s/s"), CStrUtils::FormatByteSize(value.doubleValue).strOutput.c_str());
            CStrUtils::ConsolePrintf(_T("\r\n"));
        }

        //获取网络读速度
        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_NETWORK_RECV_RATE), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("网络 接收: %sps "), CStrUtils::FormatByteSize(value.doubleValue).strOutput.c_str());
        }

        //获取网络写速度
        if (perfmon.GetFormattedCounterArray(_T(PERFM_PATH_NETWORK_SENT_RATE), PDH_FMT_DOUBLE, &value))
        {
            CStrUtils::ConsolePrintf(_T("网络 发送: %sps"), CStrUtils::FormatByteSize(value.doubleValue).strOutput.c_str());
            CStrUtils::ConsolePrintf(_T("\r\n"));
        }

        CStrUtils::ConsolePrintf(_T("\r\n"));
        ::Sleep(nTimeInterval);
    }

    return 0;
}
