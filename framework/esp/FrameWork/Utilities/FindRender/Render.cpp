#include <FrameWork/Utilities/FindRender/Render.hpp>
#include <string>
#include <FrameWork/Utilities/Misc/ToolhelpAnsi.hpp>

HWND RenderWindow = nullptr;

namespace FrameWork {

    std::vector<DWORD> GetProcessIdsByName(const std::string& ProcessName)
    {
        std::vector<DWORD> processIds;

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            return processIds;
        }

        PROCESSENTRY32A processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32A);

        if (Process32FirstA(hSnapshot, &processEntry))
        {
            do {
                if (ProcessName == std::string(processEntry.szExeFile))
                {
                    processIds.push_back(processEntry.th32ProcessID);
                }
            } while (Process32NextA(hSnapshot, &processEntry));
        }

        CloseHandle(hSnapshot);
        return processIds;
    }

    inline BOOL CALLBACK EnumChildWindowsProc(HWND hWnd, LPARAM lParam)
    {
        char windowName[256];
        GetWindowTextA(hWnd, windowName, sizeof(windowName));
        std::string name(windowName);

        char className[256];
        GetClassNameA(hWnd, className, sizeof(className));
        std::string class_str(className);

        // BlueStacks 4 (WindowsForms)
        if (name == "BlueStacks Android PluginAndroid" && class_str.find("WindowsForms10") != std::string::npos)
        {
            RenderWindow = hWnd;
            return FALSE;
        }
        // BlueStacks 4 (Qt)
        else if (name == "HD-Player" && class_str.find("Qt") != std::string::npos)
        {
            RenderWindow = hWnd;
            return FALSE;
        }
        // BlueStacks 5 / MSI App Player 5 (subWin / BlueStacksApp)
        else if (class_str == "subWin" || name == "HD-Player" || class_str.find("BlueStacks") != std::string::npos)
        {
            RECT r;
            GetClientRect(hWnd, &r);
            if ((r.right - r.left) > 300 && (r.bottom - r.top) > 300)
            {
                RenderWindow = hWnd;
                return FALSE;
            }
        }

        return TRUE;
    }

    inline BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
    {
        DWORD processId = *reinterpret_cast<DWORD*>(lParam);

        DWORD windowProcessId;
        GetWindowThreadProcessId(hWnd, &windowProcessId);

        if (windowProcessId == processId && IsWindowVisible(hWnd))
        {
            RECT r;
            GetClientRect(hWnd, &r);
            if ((r.right - r.left) > 300 && (r.bottom - r.top) > 300)
            {
                if (!RenderWindow) RenderWindow = hWnd;
                EnumChildWindows(hWnd, EnumChildWindowsProc, lParam);
            }
        }

        return TRUE;
    }

    HWND FindeRender::FindRenderWindow()
    {
        RenderWindow = nullptr;

        // 1. Try current process first (we are injected into HD-Player.exe)
        DWORD currentPid = GetCurrentProcessId();
        EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&currentPid));
        if (RenderWindow) return RenderWindow;

        // 2. Fallback to process scanning
        auto BlueStacksProcesses = GetProcessIdsByName("Bluestacks.exe");
        if (BlueStacksProcesses.empty()) {
            BlueStacksProcesses = GetProcessIdsByName("HD-Player.exe");
        }
        for (const auto& ProcessId : BlueStacksProcesses)
        {
            HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
            if (ProcessHandle)
            {
                EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&ProcessId));
                CloseHandle(ProcessHandle);
                if (RenderWindow) return RenderWindow;
            }
        }

        return RenderWindow;
    }
}
